import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  /* config options here */
  // ✅ DISABLED BOTH: Conflicts with real-time functionality
  // cacheComponents: true,
  // partialPrefetching: true,
  
  turbopack: {
    rules: {
      "*.css": {
        loaders: ["@tailwindcss/turbopack"],
        as: "*.css",
      },
    },
  },
};

export default nextConfig;
