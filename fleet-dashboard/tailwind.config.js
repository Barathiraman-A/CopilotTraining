/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'fleet-primary': '#2563eb',
        'fleet-secondary': '#7c3aed',
        'fleet-success': '#10b981',
        'fleet-warning': '#f59e0b',
        'fleet-danger': '#ef4444',
        'fleet-gray': '#6b7280'
      }
    },
  },
  plugins: [],
}
