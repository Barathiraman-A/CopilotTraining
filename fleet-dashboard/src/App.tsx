/// <reference types="vite/client" />
import { useEffect, useRef } from 'react';
import { useFleetStore } from './stores/fleetStore';
import { TelemetryWebSocketClient } from './api/websocketClient';
import { VehicleUpdateBatcher } from './utils/batcher';
import MapView from './components/MapView';
import VehicleList from './components/VehicleList';
import FilterControls from './components/FilterControls';
import ConnectionStatus from './components/ConnectionStatus';
import ErrorBoundary from './components/ErrorBoundary';

const WS_URL = import.meta.env.VITE_WS_URL || 'ws://localhost:8080/v1/telemetry/stream';
const BATCH_INTERVAL = parseInt(import.meta.env.VITE_UPDATE_BATCH_INTERVAL || '100', 10);

// Mock token - replace with actual auth
const AUTH_TOKEN = 'mock-jwt-token';

function App() {
  const wsClientRef = useRef<TelemetryWebSocketClient | null>(null);
  const batcherRef = useRef<VehicleUpdateBatcher | null>(null);
  const updateVehicles = useFleetStore(state => state.updateVehicles);
  const setConnectionStatus = useFleetStore(state => state.setConnectionStatus);

  useEffect(() => {
    // Initialize batcher
    batcherRef.current = new VehicleUpdateBatcher(BATCH_INTERVAL, (updates) => {
      updateVehicles(updates);
    });

    // Initialize WebSocket client
    wsClientRef.current = new TelemetryWebSocketClient(WS_URL, AUTH_TOKEN);

    // Handle incoming messages
    wsClientRef.current.onMessage((data) => {
      if (Array.isArray(data)) {
        batcherRef.current?.addBatch(data);
      } else {
        batcherRef.current?.addUpdate(data.id, data);
      }
    });

    // Handle connection status changes
    wsClientRef.current.onConnectionChange((status) => {
      setConnectionStatus(status);
    });

    // Handle errors
    wsClientRef.current.onError((error) => {
      console.error('WebSocket error:', error);
    });

    // Connect
    wsClientRef.current.connect();

    // Cleanup
    return () => {
      batcherRef.current?.destroy();
      wsClientRef.current?.disconnect();
    };
  }, [updateVehicles, setConnectionStatus]);

  return (
    <ErrorBoundary>
      <div className="h-screen flex flex-col bg-gray-50">
        {/* Header */}
        <header className="bg-white shadow-sm border-b border-gray-200 px-6 py-4">
          <div className="flex items-center justify-between">
            <h1 className="text-2xl font-bold text-gray-900">
              Fleet Dashboard
            </h1>
            <ConnectionStatus />
          </div>
        </header>

        {/* Main Content */}
        <div className="flex-1 flex overflow-hidden">
          {/* Left Sidebar - Filters and Vehicle List */}
          <aside className="w-96 bg-white border-r border-gray-200 flex flex-col">
            <div className="p-4 border-b border-gray-200">
              <FilterControls />
            </div>
            <div className="flex-1 overflow-hidden">
              <VehicleList />
            </div>
          </aside>

          {/* Main Map Area */}
          <main className="flex-1 relative">
            <MapView />
          </main>
        </div>
      </div>
    </ErrorBoundary>
  );
}

export default App;
