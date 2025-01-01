import { Slider } from "@/components/ui/slider";

interface AxisSliderProps {
  index: number;
  value: number;
  onValueChange: (value: number[], index: number) => void;
}

const AxisSlider = ({ index, value, onValueChange }: AxisSliderProps) => {
  return (
    <div className="mb-8 bg-gradient-to-r from-purple-50 to-blue-50 p-6 rounded-lg">
      <div className="flex justify-between mb-2">
        <label className="text-sm font-medium text-purple-900">
          Axis {index + 1}
        </label>
        <span className="text-sm text-blue-600 font-semibold">
          {value}°
        </span>
      </div>
      <Slider
        value={[value]}
        min={0}
        max={180}
        step={1}
        onValueChange={(value) => onValueChange(value, index)}
        className="w-full"
      />
    </div>
  );
};

export default AxisSlider;