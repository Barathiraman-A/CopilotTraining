import { useEffect, useRef, memo } from 'react';
import { MapContainer, TileLayer, Marker, Popup, useMap } from 'react-leaflet';
import MarkerClusterGroup from 'react-leaflet-cluster';
import L from 'leaflet';
import { useFleetStore } from '../stores/fleetStore';
import { VehicleData, VehicleStatus } from '../types';
import 'leaflet/dist/leaflet.css';

// Fix Leaflet default icon issue
delete (L.Icon.Default.prototype as any)._getIconUrl;
L.Icon.Default.mergeOptions({
  iconRetinaUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon-2x.png',
  iconUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon.png',
  shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-shadow.png',
});

// Create custom vehicle icons based on status
const createVehicleIcon = (status: VehicleStatus) => {
  const colors: Record<VehicleStatus, string> = {
    [VehicleStatus.MOVING]: '#10b981',
    [VehicleStatus.ONLINE]: '#3b82f6',
    [VehicleStatus.STOPPED]: '#f59e0b',
    [VehicleStatus.OFFLINE]: '#6b7280',
    [VehicleStatus.LOW_BATTERY]: '#ef4444',
  };

  const color = colors[status] || '#6b7280';

  return L.divIcon({
    className: 'custom-vehicle-marker',
    html: `
      <div style="
        width: 24px;
        height: 24px;
        background-color: ${color};
        border: 2px solid white;
        border-radius: 50%;
        box-shadow: 0 2px 4px rgba(0,0,0,0.3);
        display: flex;
        align-items: center;
        justify-content: center;
      ">
        <svg width="12" height="12" viewBox="0 0 20 20" fill="white">
          <path d="M8 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0zM15 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0z" />
          <path d="M3 4a1 1 0 00-1 1v10a1 1 0 001 1h1.05a2.5 2.5 0 014.9 0H10a1 1 0 001-1V5a1 1 0 00-1-1H3zM14 7a1 1 0 00-1 1v6.05A2.5 2.5 0 0115.95 16H17a1 1 0 001-1v-5a1 1 0 00-.293-.707l-2-2A1 1 0 0015 7h-1z" />
        </svg>
      </div>
    `,
    iconSize: [24, 24],
    iconAnchor: [12, 12],
    popupAnchor: [0, -12],
  });
};

interface VehicleMarkerProps {
  vehicle: VehicleData;
}

const VehicleMarker = memo(({ vehicle }: VehicleMarkerProps) => {
  const position: [number, number] = [vehicle.latitude, vehicle.longitude];
  const icon = createVehicleIcon(vehicle.status);

  return (
    <Marker position={position} icon={icon}>
      <Popup>
        <div className="p-2 min-w-[200px]">
          <h3 className="font-bold text-gray-900 mb-2">{vehicle.id}</h3>
          <div className="space-y-1 text-sm">
            <div className="flex justify-between">
              <span className="text-gray-600">Speed:</span>
              <span className="font-medium">{vehicle.speed.toFixed(1)} km/h</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-600">Battery:</span>
              <span className="font-medium">{vehicle.battery.toFixed(0)}%</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-600">Altitude:</span>
              <span className="font-medium">{vehicle.altitude?.toFixed(0) || 'N/A'} m</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-600">Satellites:</span>
              <span className="font-medium">{vehicle.satellites || 'N/A'}</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-600">Status:</span>
              <span className="font-medium capitalize">{vehicle.status.replaceAll('_', ' ').toLowerCase()}</span>
            </div>
          </div>
        </div>
      </Popup>
    </Marker>
  );
}, (prev, next) => {
  // Only re-render if relevant vehicle data displayed in the popup changed
  return prev.vehicle.latitude === next.vehicle.latitude &&
    prev.vehicle.longitude === next.vehicle.longitude &&
    prev.vehicle.status === next.vehicle.status &&
    prev.vehicle.speed === next.vehicle.speed &&
    prev.vehicle.battery === next.vehicle.battery &&
    prev.vehicle.altitude === next.vehicle.altitude &&
    prev.vehicle.satellites === next.vehicle.satellites;
});

VehicleMarker.displayName = 'VehicleMarker';

// Component to auto-fit bounds when vehicles change
function MapUpdater({ vehicles }: { vehicles: VehicleData[] }) {
  const map = useMap();
  const prevVehiclesCountRef = useRef(vehicles.length);

  useEffect(() => {
    if (vehicles.length > 0 && vehicles.length !== prevVehiclesCountRef.current) {
      const bounds = L.latLngBounds(
        vehicles.map(v => [v.latitude, v.longitude] as [number, number])
      );
      map.fitBounds(bounds, { padding: [50, 50], maxZoom: 15 });
      prevVehiclesCountRef.current = vehicles.length;
    }
  }, [vehicles, map]);

  return null;
}

function MapView() {
  const filteredVehicles = useFleetStore(state => state.getFilteredVehicles());

  // Default center (will be updated when vehicles load)
  const defaultCenter: [number, number] = [11.0168, 76.9558]; // Coimbatore, Tamil Nadu
  const defaultZoom = 12;

  return (
    <div className="h-full w-full relative" role="region" aria-label="Fleet map view">
      <MapContainer
        center={defaultCenter}
        zoom={defaultZoom}
        style={{ height: '100%', width: '100%' }}
        zoomControl={true}
      >
        {/* OpenStreetMap tiles (no API key required) */}
        <TileLayer
          attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        />

        {/* Marker clustering for performance */}
        <MarkerClusterGroup
          chunkedLoading
          maxClusterRadius={50}
          spiderfyOnMaxZoom={true}
          showCoverageOnHover={false}
          zoomToBoundsOnClick={true}
        >
          {filteredVehicles.map(vehicle => (
            <VehicleMarker key={vehicle.id} vehicle={vehicle} />
          ))}
        </MarkerClusterGroup>

        {/* Auto-fit bounds */}
        <MapUpdater vehicles={filteredVehicles} />
      </MapContainer>

      {/* Map Controls Overlay */}
      <div className="absolute top-4 right-4 z-[1000] flex flex-col gap-2">
        <div className="bg-white rounded-lg shadow-lg px-3 py-2">
          <div className="text-sm font-medium text-gray-700">
            {filteredVehicles.length} vehicle{filteredVehicles.length !== 1 ? 's' : ''} on map
          </div>
        </div>
      </div>
    </div>
  );
}

export default MapView;
