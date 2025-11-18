import { memo } from 'react';
import { FixedSizeList as List } from 'react-window';
import { useFleetStore } from '../stores/fleetStore';
import { VehicleData, VehicleStatus } from '../types';
import { formatDistanceToNow } from 'date-fns';

interface VehicleItemProps {
  vehicle: VehicleData;
  isSelected: boolean;
  onClick: () => void;
}

const VehicleItem = memo(({ vehicle, isSelected, onClick }: VehicleItemProps) => {
  const getStatusColor = (status: VehicleStatus) => {
    switch (status) {
      case VehicleStatus.MOVING:
        return '#10b981'; // Green
      case VehicleStatus.ONLINE:
        return '#3b82f6'; // Blue
      case VehicleStatus.STOPPED:
        return '#f59e0b'; // Yellow
      case VehicleStatus.OFFLINE:
        return '#6b7280'; // Gray
      case VehicleStatus.LOW_BATTERY:
        return '#ef4444'; // Red
      default:
        return '#6b7280'; // Gray
    }
  };

  const getBatteryColor = (battery: number) => {
    if (battery < 20) return 'text-red-600';
    if (battery < 50) return 'text-yellow-600';
    return 'text-green-600';
  };

  return (
    <button
      onClick={onClick}
      className={`w-full text-left px-4 py-3 border-b border-gray-200 hover:bg-gray-50 transition-colors ${
        isSelected ? 'bg-blue-50 border-l-4 border-l-fleet-primary' : ''
      }`}
      aria-label={`Vehicle ${vehicle.id}, speed ${vehicle.speed} km/h, battery ${vehicle.battery}%`}
    >
      <div className="flex items-start justify-between">
        <div className="flex-1 min-w-0">
          {/* Vehicle ID and Status */}
          <div className="flex items-center gap-2 mb-1">
            <svg className="w-5 h-5" viewBox="0 0 20 20" fill="currentColor" style={{ color: getStatusColor(vehicle.status) }}>
              <path d="M8 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0zM15 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0z" />
              <path d="M3 4a1 1 0 00-1 1v10a1 1 0 001 1h1.05a2.5 2.5 0 014.9 0H10a1 1 0 001-1V5a1 1 0 00-1-1H3zM14 7a1 1 0 00-1 1v6.05A2.5 2.5 0 0115.95 16H17a1 1 0 001-1v-5a1 1 0 00-.293-.707l-2-2A1 1 0 0015 7h-1z" />
            </svg>
            <span className="font-medium text-gray-900 truncate">
              {vehicle.id}
            </span>
          </div>

          {/* Speed */}
          <div className="text-sm text-gray-600 flex items-center gap-1">
            <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
              <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm1-12a1 1 0 10-2 0v4a1 1 0 00.293.707l2.828 2.829a1 1 0 101.415-1.415L11 9.586V6z" clipRule="evenodd" />
            </svg>
            <span>{vehicle.speed.toFixed(1)} km/h</span>
          </div>

          {/* Battery */}
          <div className={`text-sm flex items-center gap-1 ${getBatteryColor(vehicle.battery)}`}>
            <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
              <path d="M10 2a1 1 0 011 1v1a1 1 0 11-2 0V3a1 1 0 011-1zm4 8a4 4 0 11-8 0 4 4 0 018 0zm-.464 4.95l.707.707a1 1 0 001.414-1.414l-.707-.707a1 1 0 00-1.414 1.414zm2.12-10.607a1 1 0 010 1.414l-.706.707a1 1 0 11-1.414-1.414l.707-.707a1 1 0 011.414 0zM17 11a1 1 0 100-2h-1a1 1 0 100 2h1zm-7 4a1 1 0 011 1v1a1 1 0 11-2 0v-1a1 1 0 011-1zM5.05 6.464A1 1 0 106.465 5.05l-.708-.707a1 1 0 00-1.414 1.414l.707.707zm1.414 8.486l-.707.707a1 1 0 01-1.414-1.414l.707-.707a1 1 0 011.414 1.414zM4 11a1 1 0 100-2H3a1 1 0 000 2h1z" />
            </svg>
            <span>{vehicle.battery.toFixed(0)}%</span>
          </div>
        </div>

        {/* Last Update */}
        <div className="text-xs text-gray-500 ml-2">
          {formatDistanceToNow(vehicle.lastUpdate, { addSuffix: true })}
        </div>
      </div>
    </button>
  );
});

VehicleItem.displayName = 'VehicleItem';

function VehicleList() {
  const filteredVehicles = useFleetStore(state => state.getFilteredVehicles());
  
  const Row = ({ index, style }: { index: number; style: React.CSSProperties }) => {
    const vehicle = filteredVehicles[index];
    return (
      <div style={style}>
        <VehicleItem
          vehicle={vehicle}
          isSelected={false}
          onClick={() => console.log('Selected vehicle:', vehicle.id)}
        />
      </div>
    );
  };

  if (filteredVehicles.length === 0) {
    return (
      <div className="flex flex-col items-center justify-center h-full text-gray-500 p-8">
        <svg className="w-16 h-16 mb-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9.172 16.172a4 4 0 015.656 0M9 10h.01M15 10h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
        </svg>
        <p className="text-sm font-medium">No vehicles found</p>
        <p className="text-xs text-gray-400 mt-1">Try adjusting your filters</p>
      </div>
    );
  }

  return (
    <div className="h-full" role="list" aria-label="Vehicle list">
      <div className="px-4 py-2 bg-gray-50 border-b border-gray-200 text-sm text-gray-600">
        {filteredVehicles.length} vehicle{filteredVehicles.length !== 1 ? 's' : ''}
      </div>
      <List
        height={600}
        itemCount={filteredVehicles.length}
        itemSize={100}
        width="100%"
      >
        {Row}
      </List>
    </div>
  );
}

export default VehicleList;
