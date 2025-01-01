import { useState, useEffect, useCallback } from "react";
import { useNavigate } from "react-router-dom";
import { toast } from "sonner";
import { db } from "@/lib/firebase";
import { doc, onSnapshot } from "firebase/firestore";
import { Button } from "@/components/ui/button";
import { ArrowLeft } from "lucide-react";
import { storeUserDetails, initializeArmDegree, updateArmDegree } from "@/lib/firebase-operations";
import debounce from "lodash/debounce";
import AxisSlider from "./AxisSlider";
import SessionTimer from "./SessionTimer";

const RoboticArmController = () => {
  const navigate = useNavigate();
  const [axes, setAxes] = useState([90, 90, 90, 90, 90, 90]);
  const [userId, setUserId] = useState<string | null>(null);
  const [timeLeft, setTimeLeft] = useState(180);

  // Reduced debounce time to 20ms for lower latency
  const debouncedUpdateArmDegree = useCallback(
    debounce(async (index: number, value: number) => {
      try {
        await updateArmDegree(index + 1, value);
      } catch (error) {
        console.error("Error updating axis:", error);
      }
    }, 20),
    []
  );

  useEffect(() => {
    const initializeController = async () => {
      const user = localStorage.getItem("roboticArmUser");
      if (!user) {
        toast.error("Please login first");
        navigate("/");
        return;
      }

      const userObj = JSON.parse(user);
      try {
        await storeUserDetails(userObj);
        await initializeArmDegree();
        setUserId(userObj.email);
      } catch (error) {
        console.error("Initialization error:", error);
        navigate("/");
        return;
      }

      const unsubscribe = onSnapshot(
        doc(db, "roboticArms", "armDegree"),
        (snapshot) => {
          if (snapshot.exists()) {
            const data = snapshot.data();
            const newAxes = [
              data.axis1 ?? 90,
              data.axis2 ?? 90,
              data.axis3 ?? 90,
              data.axis4 ?? 90,
              data.axis5 ?? 90,
              data.axis6 ?? 90,
            ];
            setAxes(newAxes);
          }
        },
        (error) => {
          console.error("Firestore subscription error:", error);
          toast.error("Error connecting to database");
        }
      );

      const timer = setInterval(() => {
        setTimeLeft((prevTime) => {
          if (prevTime <= 1) {
            clearInterval(timer);
            toast.info("Session expired");
            navigate("/");
            return 0;
          }
          return prevTime - 1;
        });
      }, 1000);

      return () => {
        unsubscribe();
        clearInterval(timer);
        debouncedUpdateArmDegree.cancel();
      };
    };

    initializeController();
  }, [navigate, debouncedUpdateArmDegree]);

  const handleAxisChange = (value: number[], index: number) => {
    if (!userId) return;
    
    const newAxes = [...axes];
    newAxes[index] = value[0];
    setAxes(newAxes);
    
    debouncedUpdateArmDegree(index, value[0]);
  };

  return (
    <div className="space-y-8 w-full max-w-2xl p-8 rounded-lg bg-white shadow-lg">
      <div className="flex justify-between items-center mb-6">
        <Button
          variant="outline"
          onClick={() => navigate("/")}
          className="flex items-center gap-2"
        >
          <ArrowLeft className="h-4 w-4" />
          Back
        </Button>
        <SessionTimer timeLeft={timeLeft} />
      </div>
      <div className="text-left">
        <h2 className="text-2xl font-bold mb-6 text-purple-700">Robotic Arm Controller</h2>
        {axes.map((value, index) => (
          <AxisSlider
            key={index}
            index={index}
            value={value}
            onValueChange={handleAxisChange}
          />
        ))}
      </div>
    </div>
  );
};

export default RoboticArmController;