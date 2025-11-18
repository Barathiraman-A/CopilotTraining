import { useFleetStore } from '../stores/fleetStore';
import { VehicleStatus, TimeRange } from '../types';

function FilterControls() {
  const filters = useFleetStore(state => state.filters);
  const setFilter = useFleetStore(state => state.setFilter);
  const lowBatteryCount = useFleetStore(state => state.getLowBatteryCount());

  return (
    <div className="space-y-4" role="search" aria-label="Filter vehicles">
      <h2 className="text-lg font-semibold text-gray-900">Filters</h2>

      {/* Status Filter */}
      <div>
        <label 
          htmlFor="status-filter"
          className="block text-sm font-medium text-gray-700 mb-1"
        >
          Status
        </label>
        <select
          id="status-filter"
          value={filters.status}
          onChange={(e) => setFilter('status', e.target.value as VehicleStatus | 'all')}
          className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-fleet-primary focus:border-fleet-primary"
          aria-label="Filter by vehicle status"
        >
          <option value="all">All Vehicles</option>
          <option value={VehicleStatus.ONLINE}>Online</option>
          <option value={VehicleStatus.MOVING}>Moving</option>
          <option value={VehicleStatus.STOPPED}>Stopped</option>
          <option value={VehicleStatus.OFFLINE}>Offline</option>
        </select>
      </div>

      {/* Search Input */}
      <div>
        <label 
          htmlFor="search-input"
          className="block text-sm font-medium text-gray-700 mb-1"
        >
          Search
        </label>
        <input
          id="search-input"
          type="text"
          placeholder="Search by Vehicle ID..."
          value={filters.searchQuery}
          onChange={(e) => setFilter('searchQuery', e.target.value)}
          className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-fleet-primary focus:border-fleet-primary"
          aria-label="Search vehicles by ID"
        />
      </div>

      {/* Low Battery Filter */}
      <div className="flex items-center">
        <input
          id="low-battery-filter"
          type="checkbox"
          checked={filters.lowBatteryOnly}
          onChange={(e) => setFilter('lowBatteryOnly', e.target.checked)}
          className="w-4 h-4 text-fleet-primary border-gray-300 rounded focus:ring-2 focus:ring-fleet-primary"
          aria-label="Show only low battery vehicles"
        />
        <label 
          htmlFor="low-battery-filter"
          className="ml-2 text-sm text-gray-700"
        >
          Low Battery Only
          {lowBatteryCount > 0 && (
            <span className="ml-1 px-2 py-0.5 bg-red-100 text-red-800 text-xs font-medium rounded-full">
              {lowBatteryCount}
            </span>
          )}
        </label>
      </div>

      {/* Time Range */}
      <div>
        <label className="block text-sm font-medium text-gray-700 mb-2">
          Time Range
        </label>
        <div className="flex flex-wrap gap-2">
          {[
            { value: TimeRange.LAST_HOUR, label: 'Last Hour' },
            { value: TimeRange.LAST_24H, label: 'Last 24h' },
            { value: TimeRange.LAST_WEEK, label: 'Last Week' }
          ].map(({ value, label }) => (
            <button
              key={value}
              onClick={() => setFilter('timeRange', value)}
              className={`px-3 py-1.5 text-sm font-medium rounded-lg transition-colors ${
                filters.timeRange === value
                  ? 'bg-fleet-primary text-white'
                  : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
              }`}
              aria-pressed={filters.timeRange === value}
              aria-label={`Select ${label} time range`}
            >
              {label}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}

export default FilterControls;
