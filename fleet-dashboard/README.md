# Fleet Dashboard

Real-time fleet monitoring dashboard with vehicle telemetry visualization. Built with React, TypeScript, and Leaflet for high-performance monitoring of 1000+ vehicles.

## Features

- **Real-time Vehicle Tracking**: WebSocket-based live updates with automatic reconnection
- **Interactive Map**: Leaflet map with marker clustering for 10,000+ vehicles
- **Performance Optimized**: Update batching, virtualized lists, React.memo optimization
- **Advanced Filtering**: Filter by status, search by ID, low battery alerts, time range selection
- **Accessibility**: WCAG 2.1 AA compliant with keyboard navigation and screen reader support
- **Error Handling**: Graceful degradation, connection status monitoring, error boundaries

## Technology Stack

- **React 18.2+** with TypeScript 5.3+
- **Zustand 4.4+** for state management
- **Leaflet 1.9+** with react-leaflet for maps
- **Chart.js 4.4+** for data visualization
- **Tailwind CSS 3.4+** for styling
- **Vite 5.0+** for build tooling
- **react-window** for virtualized lists

## Getting Started

### Prerequisites

- Node.js 18+ and npm 9+

### Installation

```bash
# Install dependencies
npm install

# Copy environment configuration
cp .env.example .env

# Edit .env with your API endpoints
# VITE_API_URL=http://your-api-url
# VITE_WS_URL=ws://your-websocket-url
```

### Development

```bash
# Start development server
npm run dev

# Open http://localhost:3000
```

### Build

```bash
# Build for production
npm run build

# Preview production build
npm run preview
```

## Project Structure

```
fleet-dashboard/
├── src/
│   ├── api/
│   │   ├── websocketClient.ts    # WebSocket client with reconnection
│   │   └── restClient.ts          # REST API wrapper
│   ├── components/
│   │   ├── App.tsx                # Main application
│   │   ├── MapView.tsx            # Leaflet map with clustering
│   │   ├── VehicleList.tsx        # Virtualized vehicle list
│   │   ├── FilterControls.tsx     # Search and filter UI
│   │   ├── ConnectionStatus.tsx   # Connection indicator
│   │   └── ErrorBoundary.tsx      # Error handling
│   ├── stores/
│   │   └── fleetStore.ts          # Zustand state management
│   ├── types/
│   │   └── index.ts               # TypeScript definitions
│   ├── utils/
│   │   └── batcher.ts             # Update batching utility
│   ├── main.tsx                   # React entry point
│   └── index.css                  # Global styles
├── package.json
├── tsconfig.json
├── vite.config.ts
└── tailwind.config.js
```

## Performance Targets

- **Initial Load**: <3s
- **WebSocket Latency**: <2s
- **Frame Rate**: 60 FPS with 1000+ vehicles
- **Bundle Size**: <500 KB gzipped
- **Memory Usage**: <150 MB

## API Integration

### WebSocket Messages

Connect to `ws://api.example.com/v1/telemetry/stream?token=YOUR_TOKEN`

**Vehicle Update Message:**
```json
{
  "type": "vehicle_update",
  "data": {
    "id": "VEH-001",
    "speed": 65.5,
    "battery": 87.2,
    "latitude": 37.7749,
    "longitude": -122.4194,
    "altitude": 15,
    "satellites": 8,
    "timestamp": "2025-11-18T10:30:00Z",
    "status": "moving"
  }
}
```

### REST API Endpoints

- `GET /api/fleet/vehicles` - List all vehicles
- `GET /api/vehicles/{id}` - Get vehicle details
- `GET /api/vehicles/{id}/telemetry?start=...&end=...` - Historical data
- `GET /api/fleet/analytics?start=...&end=...` - Fleet analytics

## Accessibility

- ARIA labels on all interactive elements
- Keyboard navigation (Tab, Enter, Escape)
- Screen reader support
- Color + pattern indicators for color blindness
- Focus indicators on all focusable elements

## Browser Support

- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

## License

Proprietary - All rights reserved

## Support

For issues or questions, contact the development team.
