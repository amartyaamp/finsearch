/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'standalone',
  async rewrites() {
    const backendUrl = process.env.API_URL || 'http://localhost:8000';
    return [
      {
        source: '/api/backend/:path*',
        destination: `${backendUrl}/:path*` // Proxies to FastAPI
      }
    ];
  }
};

module.exports = nextConfig;
