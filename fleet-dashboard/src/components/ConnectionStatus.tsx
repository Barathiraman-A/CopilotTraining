import { useFleetStore } from '../stores/fleetStore';
import { formatDistanceToNow } from 'date-fns';

function ConnectionStatus() {
  const connectionStatus = useFleetStore(state => state.connectionStatus);
  const onlineCount = useFleetStore(state => state.getOnlineCount());
  const totalCount = useFleetStore(state => state.vehicles.size);

  const getStatusColor = () => {
    if (!connectionStatus.connected) return 'bg-red-500';
    if (connectionStatus.stale) return 'bg-yellow-500';
    return 'bg-green-500';
  };

  const getStatusText = () => {
    if (connectionStatus.reconnecting) return 'Reconnecting...';
    if (!connectionStatus.connected) return 'Disconnected';
    if (connectionStatus.stale) return 'Connection Stale';
    return 'Connected';
  };

  const getStatusIcon = () => {
    if (!connectionStatus.connected) {
      return (
        <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
          <path fillRule="evenodd" d="M13.477 14.89A6 6 0 015.11 6.524l8.367 8.368zm1.414-1.414L6.524 5.11a6 6 0 018.367 8.367zM18 10a8 8 0 11-16 0 8 8 0 0116 0z" clipRule="evenodd" />
        </svg>
      );
    }
    if (connectionStatus.stale) {
      return (
        <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
          <path fillRule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z" clipRule="evenodd" />
        </svg>
      );
    }
    return (
      <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
        <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clipRule="evenodd" />
      </svg>
    );
  };

  return (
    <div className="flex items-center gap-4">
      {/* Vehicle Count */}
      <div className="flex items-center gap-2 text-sm text-gray-600">
        <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 20 20">
          <path d="M8 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0zM15 16.5a1.5 1.5 0 11-3 0 1.5 1.5 0 013 0z" />
          <path d="M3 4a1 1 0 00-1 1v10a1 1 0 001 1h1.05a2.5 2.5 0 014.9 0H10a1 1 0 001-1V5a1 1 0 00-1-1H3zM14 7a1 1 0 00-1 1v6.05A2.5 2.5 0 0115.95 16H17a1 1 0 001-1v-5a1 1 0 00-.293-.707l-2-2A1 1 0 0015 7h-1z" />
        </svg>
        <span className="font-medium">{onlineCount}</span>
        <span>/</span>
        <span>{totalCount}</span>
        <span className="hidden sm:inline">vehicles</span>
      </div>

      {/* Connection Status */}
      <div 
        className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-gray-100"
        role="status"
        aria-live="polite"
        aria-label={`Connection status: ${getStatusText()}`}
      >
        <div className={`w-2 h-2 rounded-full ${getStatusColor()} animate-pulse`} />
        <span className="text-sm font-medium text-gray-700">
          {getStatusText()}
        </span>
        <div className="text-gray-500">
          {getStatusIcon()}
        </div>
      </div>

      {/* Last Update Time */}
      {connectionStatus.lastUpdate > 0 && (
        <div className="text-xs text-gray-500 hidden md:block">
          Updated{' '}
          {formatDistanceToNow(connectionStatus.lastUpdate, { addSuffix: true })}
        </div>
      )}
    </div>
  );
}

export default ConnectionStatus;
