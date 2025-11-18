import { HistoricalDataRequest, HistoricalDataResponse, VehicleData } from '../types';

export class RestApiClient {
  private baseUrl: string;
  private token: string;

  constructor(baseUrl: string, token: string) {
    this.baseUrl = baseUrl;
    this.token = token;
  }

  private async fetchWithAuth(url: string, options: RequestInit = {}): Promise<Response> {
    const headers = {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${this.token}`,
      ...options.headers,
    };

    const response = await fetch(url, { ...options, headers });

    if (!response.ok) {
      const error = await response.json().catch(() => ({ message: 'Unknown error' }));
      throw new Error(error.message || `HTTP ${response.status}: ${response.statusText}`);
    }

    return response;
  }

  public async getVehicles(): Promise<VehicleData[]> {
    const response = await this.fetchWithAuth(`${this.baseUrl}/api/fleet/vehicles`);
    const data = await response.json();
    return data.vehicles || [];
  }

  public async getVehicle(vehicleId: string): Promise<VehicleData> {
    const response = await this.fetchWithAuth(`${this.baseUrl}/api/vehicles/${vehicleId}`);
    return response.json();
  }

  public async getHistoricalData(request: HistoricalDataRequest): Promise<HistoricalDataResponse> {
    const params = new URLSearchParams({
      start: request.start,
      end: request.end,
      ...(request.interval && { interval: request.interval.toString() })
    });

    const response = await this.fetchWithAuth(
      `${this.baseUrl}/api/vehicles/${request.vehicleId}/telemetry?${params}`
    );
    return response.json();
  }

  public async getFleetAnalytics(start: string, end: string): Promise<FleetAnalytics> {
    const params = new URLSearchParams({ start, end });
    const response = await this.fetchWithAuth(
      `${this.baseUrl}/api/fleet/analytics?${params}`
    );
    return response.json();
  }
}

export interface FleetAnalytics {
  totalVehicles: number;
  onlineVehicles: number;
  movingVehicles: number;
  lowBatteryVehicles: number;
  averageSpeed: number;
  averageBattery: number;
  totalDistance: number;
}
