interface SessionTimerProps {
  timeLeft: number;
}

const SessionTimer = ({ timeLeft }: SessionTimerProps) => {
  const formatTime = (seconds: number) => {
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = seconds % 60;
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  return (
    <div className="text-lg font-semibold text-purple-700">
      Time remaining: {formatTime(timeLeft)}
    </div>
  );
};

export default SessionTimer;