#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Define color schemes for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

IMAGE_NAME="fin_search:local"
CONTAINER_NAME="fin_search_ci"

# Cleanup function to ensure no artifacts are left behind
cleanup() {
    EXIT_CODE=$?
    echo -e "\n${BLUE}🧹 Cleaning up Docker resources...${NC}"
    
    # Stop and remove the container if it exists
    if [ "$(docker ps -aq -f name=$CONTAINER_NAME)" ]; then
        echo -e "Stopping and removing container: ${CONTAINER_NAME}"
        docker stop $CONTAINER_NAME >/dev/null 2>&1 || true
        docker rm -f $CONTAINER_NAME >/dev/null 2>&1 || true
    fi
    
    # Remove the built image
    if [ "$(docker images -q $IMAGE_NAME)" ]; then
        echo -e "Removing image: ${IMAGE_NAME}"
        docker rmi $IMAGE_NAME >/dev/null 2>&1 || true
    fi
    
    if [ $EXIT_CODE -ne 0 ]; then
        echo -e "${RED}❌ Local CI failed with exit code $EXIT_CODE${NC}"
    fi
}

# Register the cleanup function to be called on any exit
trap cleanup EXIT

echo -e "${BLUE}🚀 Building Docker image for local testing...${NC}"

# Build the docker image and tag it
docker build -t $IMAGE_NAME .

echo -e "${GREEN}✅ Build complete! Running containerized tests...${NC}"

# Run the docker container with a name so we can reference it in cleanup.
# The Dockerfile's CMD/ENTRYPOINT is expected to run the tests.
docker run --name $CONTAINER_NAME $IMAGE_NAME

echo -e "${GREEN}🎉 All local CI tests passed successfully!${NC}"
