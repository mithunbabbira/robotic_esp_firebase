import { db, rtdb } from "@/lib/firebase";
import { doc, setDoc } from "firebase/firestore";
import { ref, set, update } from "firebase/database";
import { toast } from "sonner";

interface UserDetails {
  name: string;
  email: string;
  lastLogin: string;
}

export const storeUserDetails = async (userObj: { name: string; email: string }) => {
  try {
    const userDetailsRef = doc(db, "roboticArms", "userDetails");
    await setDoc(userDetailsRef, {
      [`users.${userObj.email.replace(/\./g, "_")}`]: {
        name: userObj.name,
        email: userObj.email,
        lastLogin: new Date().toISOString(),
      }
    }, { merge: true });

    const rtdbUserRef = ref(rtdb, `roboticArms/userDetails/${userObj.email.replace(/\./g, "_")}`);
    await set(rtdbUserRef, {
      name: userObj.name,
      email: userObj.email,
      lastLogin: new Date().toISOString(),
    });
  } catch (error) {
    console.error("Error storing user details:", error);
    toast.error("Error storing user details");
    throw error;
  }
};

export const initializeArmDegree = async () => {
  try {
    const initialDegrees = {
      axis1: 90,
      axis2: 90,
      axis3: 90,
      axis4: 90,
      axis5: 90,
      axis6: 90,
      lastUpdated: new Date().toISOString(),
    };

    // Update Firestore
    await setDoc(doc(db, "roboticArms", "armDegree"), initialDegrees);
    
    // Optimize RTDB update by using update
    const updates: { [key: string]: any } = {};
    Object.entries(initialDegrees).forEach(([key, value]) => {
      updates[`roboticArms/armDegree/${key}`] = value;
    });
    await update(ref(rtdb), updates);
  } catch (error) {
    console.error("Error initializing arm degrees:", error);
    toast.error("Error initializing controller");
    throw error;
  }
};

export const updateArmDegree = async (axisNumber: number, value: number) => {
  try {
    const timestamp = new Date().toISOString();
    
    // Update Firestore
    await setDoc(doc(db, "roboticArms", "armDegree"), {
      [`axis${axisNumber}`]: value,
      lastUpdated: timestamp,
    }, { merge: true });

    // Optimize RTDB update by using a more direct path
    const updates: { [key: string]: any } = {};
    updates[`roboticArms/armDegree/axis${axisNumber}`] = value;
    updates['roboticArms/armDegree/lastUpdated'] = timestamp;
    
    // Use ref(rtdb) directly for a more efficient update
    await update(ref(rtdb), updates);
  } catch (error) {
    console.error("Error updating arm degree:", error);
    toast.error("Failed to update position");
    throw error;
  }
};