#!/bin/bash
set -e

# Define color schemes for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 Building Docker image for local testing...${NC}"

# Build the docker image and tag it as 'fin_search:local'
docker build -t fin_search:local .

echo -e "${GREEN}✅ Build complete! Running containerized tests...${NC}"

# Run the docker container. The Dockerfile's CMD automatically runs the tests.
# The --rm flag ensures the container is cleaned up after execution.
docker run --rm fin_search:local

echo -e "${GREEN}🎉 All local CI tests passed successfully!${NC}"
