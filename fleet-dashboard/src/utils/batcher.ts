import { VehicleData } from '../types';

export class VehicleUpdateBatcher {
  private pendingUpdates: Map<string, VehicleData> = new Map();
  private flushTimer: number | null = null;
  private flushInterval: number;
  private flushCallback: (updates: Map<string, VehicleData>) => void;

  constructor(flushInterval: number, flushCallback: (updates: Map<string, VehicleData>) => void) {
    this.flushInterval = flushInterval;
    this.flushCallback = flushCallback;
  }

  public addUpdate(vehicleId: string, data: VehicleData): void {
    this.pendingUpdates.set(vehicleId, data);

    if (!this.flushTimer) {
      this.flushTimer = window.setTimeout(() => this.flush(), this.flushInterval);
    }
  }

  public addBatch(updates: VehicleData[]): void {
    updates.forEach(update => {
      this.pendingUpdates.set(update.id, update);
    });

    if (!this.flushTimer) {
      this.flushTimer = window.setTimeout(() => this.flush(), this.flushInterval);
    }
  }

  public flush(): void {
    if (this.pendingUpdates.size > 0) {
      this.flushCallback(new Map(this.pendingUpdates));
      this.pendingUpdates.clear();
    }

    if (this.flushTimer) {
      clearTimeout(this.flushTimer);
      this.flushTimer = null;
    }
  }

  public destroy(): void {
    if (this.flushTimer) {
      clearTimeout(this.flushTimer);
      this.flushTimer = null;
    }
    this.pendingUpdates.clear();
  }
}
