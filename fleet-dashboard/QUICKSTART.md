# Quick Start Guide

## Installation (2 minutes)

### Option 1: Using Setup Script (Recommended)
```powershell
cd c:\git\Copilot\fleet-dashboard
.\setup.ps1
```

### Option 2: Manual Installation
```powershell
cd c:\git\Copilot\fleet-dashboard
npm install
cp .env.example .env
# Edit .env with your API URLs
```

## Configuration

Edit `.env` file:
```env
VITE_API_URL=http://localhost:8080
VITE_WS_URL=ws://localhost:8080/v1/telemetry/stream
VITE_AUTH_ENABLED=true
```

## Running the Application

```powershell
# Development mode (with hot reload)
npm run dev

# Open http://localhost:3000
```

## Building for Production

```powershell
# Create optimized production build
npm run build

# Preview production build
npm run preview
```

## Testing with Mock Data

If you don't have a backend yet, you can test with mock WebSocket data:

1. Install a WebSocket test tool:
```powershell
npm install -g wscat
```

2. Create a mock WebSocket server (in a separate terminal):
```powershell
wscat -l 8080
```

3. Send mock vehicle updates:
```json
{"type":"vehicle_update","data":{"id":"VEH-001","speed":65.5,"battery":87.2,"latitude":37.7749,"longitude":-122.4194,"altitude":15,"satellites":8,"timestamp":"2025-11-18T10:30:00Z","status":"moving"},"timestamp":"2025-11-18T10:30:00Z"}
```

## Troubleshooting

### Error: "Cannot find module 'react'"
**Solution**: Run `npm install`

### Error: Port 3000 already in use
**Solution**: Change port in `vite.config.ts` or kill the process:
```powershell
netstat -ano | findstr :3000
taskkill /PID <PID> /F
```

### TypeScript errors in editor
**Solution**: These are expected before installation. Run `npm install` to resolve.

### Map not showing
**Solution**: Check browser console for errors. Ensure internet connection for OpenStreetMap tiles.

### WebSocket not connecting
**Solution**: 
1. Verify `VITE_WS_URL` in `.env`
2. Check backend is running
3. Check browser console for connection errors

## Project Structure Overview

```
fleet-dashboard/
├── src/
│   ├── api/              # WebSocket & REST clients
│   ├── components/       # React components
│   ├── stores/           # Zustand state management
│   ├── types/            # TypeScript definitions
│   ├── utils/            # Helper utilities
│   └── App.tsx           # Main application
├── package.json          # Dependencies
├── vite.config.ts        # Build configuration
└── README.md             # Full documentation
```

## Key npm Scripts

```powershell
npm run dev      # Start development server
npm run build    # Build for production
npm run preview  # Preview production build
npm run lint     # Run ESLint
```

## Browser DevTools Tips

### View WebSocket Messages
1. Open DevTools (F12)
2. Network tab → WS filter
3. Click on websocket connection
4. View Messages tab

### Monitor Performance
1. Open DevTools (F12)
2. Performance tab → Record
3. Interact with map/filters
4. Stop recording → analyze frame rate

### Check Memory Usage
1. Open DevTools (F12)
2. Memory tab → Take snapshot
3. Interact with application
4. Take another snapshot → compare

## Next Steps

1. ✅ Install dependencies
2. ✅ Configure environment variables
3. ✅ Start development server
4. 🔄 Integrate with backend API
5. 🔄 Add authentication
6. 🔄 Deploy to production

For detailed documentation, see **README.md** and **IMPLEMENTATION_SUMMARY.md**.
