import { VehicleData, WebSocketMessage, ConnectionStatus } from '../types';

type MessageHandler = (data: VehicleData | VehicleData[]) => void;
type ConnectionHandler = (status: ConnectionStatus) => void;
type ErrorHandler = (error: ErrorData) => void;

interface ErrorData {
  code: string;
  message: string;
}

export class TelemetryWebSocketClient {
  private ws: WebSocket | null = null;
  private url: string;
  private token: string;
  private reconnectAttempts: number = 0;
  private maxReconnectAttempts: number = 10;
  private reconnectDelay: number = 1000;
  private reconnectTimer: number | null = null;
  private messageHandlers: Set<MessageHandler> = new Set();
  private connectionHandlers: Set<ConnectionHandler> = new Set();
  private errorHandlers: Set<ErrorHandler> = new Set();
  private pingInterval: number | null = null;
  private lastMessageTime: number = Date.now();
  private staleCheckInterval: number | null = null;

  constructor(url: string, token: string) {
    this.url = url;
    this.token = token;
  }

  public connect(): void {
    try {
      const wsUrl = `${this.url}?token=${this.token}`;
      this.ws = new WebSocket(wsUrl);

      this.ws.onopen = this.handleOpen.bind(this);
      this.ws.onmessage = this.handleMessage.bind(this);
      this.ws.onerror = this.handleError.bind(this);
      this.ws.onclose = this.handleClose.bind(this);
    } catch (error) {
      console.error('WebSocket connection error:', error);
      this.handleReconnect();
    }
  }

  public disconnect(): void {
    this.clearTimers();
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
    this.reconnectAttempts = 0;
  }

  public onMessage(handler: MessageHandler): () => void {
    this.messageHandlers.add(handler);
    return () => this.messageHandlers.delete(handler);
  }

  public onConnectionChange(handler: ConnectionHandler): () => void {
    this.connectionHandlers.add(handler);
    return () => this.connectionHandlers.delete(handler);
  }

  public onError(handler: ErrorHandler): () => void {
    this.errorHandlers.add(handler);
    return () => this.errorHandlers.delete(handler);
  }

  private handleOpen(): void {
    console.log('WebSocket connected');
    this.reconnectAttempts = 0;
    this.lastMessageTime = Date.now();
    this.notifyConnectionChange({
      connected: true,
      reconnecting: false,
      lastUpdate: Date.now(),
      stale: false
    });

    // Start ping to keep connection alive
    this.pingInterval = window.setInterval(() => {
      if (this.ws && this.ws.readyState === WebSocket.OPEN) {
        this.ws.send(JSON.stringify({ type: 'ping' }));
      }
    }, 30000); // Ping every 30 seconds

    // Check for stale connection (no messages for >10s)
    this.staleCheckInterval = window.setInterval(() => {
      const timeSinceLastMessage = Date.now() - this.lastMessageTime;
      const isStale = timeSinceLastMessage > 10000;
      
      this.notifyConnectionChange({
        connected: this.ws?.readyState === WebSocket.OPEN || false,
        reconnecting: false,
        lastUpdate: this.lastMessageTime,
        stale: isStale
      });
    }, 5000); // Check every 5 seconds
  }

  private handleMessage(event: MessageEvent): void {
    try {
      const message: WebSocketMessage = JSON.parse(event.data);
      this.lastMessageTime = Date.now();

      switch (message.type) {
        case 'vehicle_update':
          this.messageHandlers.forEach(handler => handler(message.data as VehicleData));
          break;
        case 'batch_update':
          this.messageHandlers.forEach(handler => handler(message.data as VehicleData[]));
          break;
        case 'error':
          this.errorHandlers.forEach(handler => handler(message.data as ErrorData));
          break;
        case 'connection':
          // Handle connection acknowledgment
          break;
      }
    } catch (error) {
      console.error('Failed to parse WebSocket message:', error);
    }
  }

  private handleError(event: Event): void {
    console.error('WebSocket error:', event);
    this.errorHandlers.forEach(handler => 
      handler({ code: 'WS_ERROR', message: 'WebSocket connection error' })
    );
  }

  private handleClose(): void {
    console.log('WebSocket closed');
    this.clearTimers();
    this.notifyConnectionChange({
      connected: false,
      reconnecting: this.reconnectAttempts < this.maxReconnectAttempts,
      lastUpdate: this.lastMessageTime,
      stale: true
    });
    this.handleReconnect();
  }

  private handleReconnect(): void {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.error('Max reconnection attempts reached');
      this.errorHandlers.forEach(handler =>
        handler({ code: 'MAX_RECONNECT', message: 'Failed to reconnect after maximum attempts' })
      );
      return;
    }

    this.reconnectAttempts++;
    const delay = Math.min(
      this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1),
      30000
    );

    console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);
    
    this.notifyConnectionChange({
      connected: false,
      reconnecting: true,
      lastUpdate: this.lastMessageTime,
      stale: true
    });

    this.reconnectTimer = window.setTimeout(() => {
      this.connect();
    }, delay);
  }

  private notifyConnectionChange(status: ConnectionStatus): void {
    this.connectionHandlers.forEach(handler => handler(status));
  }

  private clearTimers(): void {
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    if (this.pingInterval) {
      clearInterval(this.pingInterval);
      this.pingInterval = null;
    }
    if (this.staleCheckInterval) {
      clearInterval(this.staleCheckInterval);
      this.staleCheckInterval = null;
    }
  }
}
