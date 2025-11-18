// Vehicle telemetry data types
export interface VehicleData {
  id: string;
  speed: number; // km/h
  battery: number; // percentage 0-100
  latitude: number;
  longitude: number;
  altitude?: number;
  satellites?: number;
  timestamp: string; // ISO 8601
  status: VehicleStatus;
  lastUpdate: number; // Unix timestamp ms
}

export enum VehicleStatus {
  ONLINE = 'online',
  MOVING = 'moving',
  STOPPED = 'stopped',
  OFFLINE = 'offline',
  LOW_BATTERY = 'low_battery'
}

export interface WebSocketMessage {
  type: 'vehicle_update' | 'batch_update' | 'error' | 'connection';
  data: VehicleData | VehicleData[] | ErrorData | ConnectionData;
  timestamp: string;
}

export interface ErrorData {
  code: string;
  message: string;
}

export interface ConnectionData {
  status: 'connected' | 'disconnected' | 'reconnecting';
  clientId?: string;
}

export interface HistoricalDataRequest {
  vehicleId: string;
  start: string; // ISO 8601
  end: string; // ISO 8601
  interval?: number; // seconds
}

export interface HistoricalDataResponse {
  vehicleId: string;
  data: VehicleData[];
  count: number;
}

export interface FilterState {
  status: VehicleStatus | 'all';
  searchQuery: string;
  lowBatteryOnly: boolean;
  timeRange: TimeRange;
}

export enum TimeRange {
  LAST_HOUR = 'last_hour',
  LAST_24H = 'last_24h',
  LAST_WEEK = 'last_week',
  CUSTOM = 'custom'
}

export interface ConnectionStatus {
  connected: boolean;
  reconnecting: boolean;
  lastUpdate: number;
  stale: boolean; // No updates received for >10s
}
