import { create } from 'zustand';
import { VehicleData, FilterState, VehicleStatus, TimeRange, ConnectionStatus } from '../types';

interface FleetStore {
  // Vehicle data
  vehicles: Map<string, VehicleData>;
  
  // Filters
  filters: FilterState;
  
  // Connection status
  connectionStatus: ConnectionStatus;
  
  // Actions
  updateVehicle: (vehicleId: string, data: VehicleData) => void;
  updateVehicles: (updates: Map<string, VehicleData>) => void;
  setFilter: <K extends keyof FilterState>(key: K, value: FilterState[K]) => void;
  setConnectionStatus: (status: ConnectionStatus) => void;
  clearVehicles: () => void;
  
  // Computed/Derived data
  getFilteredVehicles: () => VehicleData[];
  getVehicleById: (id: string) => VehicleData | undefined;
  getVehiclesByStatus: (status: VehicleStatus) => VehicleData[];
  getOnlineCount: () => number;
  getLowBatteryCount: () => number;
}

export const useFleetStore = create<FleetStore>((set, get) => ({
  vehicles: new Map(),
  
  filters: {
    status: 'all',
    searchQuery: '',
    lowBatteryOnly: false,
    timeRange: TimeRange.LAST_HOUR
  },
  
  connectionStatus: {
    connected: false,
    reconnecting: false,
    lastUpdate: Date.now(),
    stale: true
  },

  updateVehicle: (vehicleId: string, data: VehicleData) => {
    set((state) => {
      const vehicles = new Map(state.vehicles);
      const existing = vehicles.get(vehicleId);
      
      vehicles.set(vehicleId, {
        ...existing,
        ...data,
        lastUpdate: Date.now()
      });
      
      return { vehicles };
    });
  },

  updateVehicles: (updates: Map<string, VehicleData>) => {
    set((state) => {
      const vehicles = new Map(state.vehicles);
      const now = Date.now();
      
      updates.forEach((data, vehicleId) => {
        const existing = vehicles.get(vehicleId);
        vehicles.set(vehicleId, {
          ...existing,
          ...data,
          lastUpdate: now
        });
      });
      
      return { vehicles };
    });
  },

  setFilter: (key, value) => {
    set((state) => ({
      filters: {
        ...state.filters,
        [key]: value
      }
    }));
  },

  setConnectionStatus: (status: ConnectionStatus) => {
    set({ connectionStatus: status });
  },

  clearVehicles: () => {
    set({ vehicles: new Map() });
  },

  getFilteredVehicles: () => {
    const { vehicles, filters } = get();
    let filtered = Array.from(vehicles.values());

    // Filter by status
    if (filters.status !== 'all') {
      filtered = filtered.filter(v => v.status === filters.status);
    }

    // Filter by low battery
    if (filters.lowBatteryOnly) {
      filtered = filtered.filter(v => v.battery < 20);
    }

    // Filter by search query
    if (filters.searchQuery) {
      const query = filters.searchQuery.toLowerCase();
      filtered = filtered.filter(v => 
        v.id.toLowerCase().includes(query)
      );
    }

    return filtered;
  },

  getVehicleById: (id: string) => {
    return get().vehicles.get(id);
  },

  getVehiclesByStatus: (status: VehicleStatus) => {
    const vehicles = Array.from(get().vehicles.values());
    return vehicles.filter(v => v.status === status);
  },

  getOnlineCount: () => {
    const vehicles = Array.from(get().vehicles.values());
    return vehicles.filter(v => 
      v.status === VehicleStatus.ONLINE || v.status === VehicleStatus.MOVING
    ).length;
  },

  getLowBatteryCount: () => {
    const vehicles = Array.from(get().vehicles.values());
    return vehicles.filter(v => v.battery < 20).length;
  }
}));
