import type { Metadata } from 'next';
import './globals.css';

export const metadata: Metadata = {
  title: 'FinSearch — Financial Pattern Search Engine',
  description:
    'Find historical stock price patterns that match the current market shape using FAISS vector search, Z-score normalisation, and live Yahoo Finance data.',
  keywords: ['stock pattern search', 'FAISS', 'technical analysis', 'NSE', 'BSE', 'quantitative finance'],
  openGraph: {
    title: 'FinSearch — Financial Pattern Search Engine',
    description: 'Find historical stock patterns that match today\'s market shape.',
    type: 'website',
  },
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en">
      <head>
        <link rel="preconnect" href="https://fonts.googleapis.com" />
        <link rel="preconnect" href="https://fonts.gstatic.com" crossOrigin="anonymous" />
        <link
          href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600;700&display=swap"
          rel="stylesheet"
        />
      </head>
      <body>{children}</body>
    </html>
  );
}
