import { initializeApp } from "firebase/app";
import { getFirestore } from "firebase/firestore";
import { getDatabase } from "firebase/database";

const firebaseConfig = {
  apiKey: "AIzaSyCcf7QnocO_XnRpU2fP5wfhS_nPRuJKpHs",
  authDomain: "robotic-arm-38d44.firebaseapp.com",
  projectId: "robotic-arm-38d44",
  storageBucket: "robotic-arm-38d44.firebasestorage.app",
  messagingSenderId: "63240445660",
  appId: "1:63240445660:web:eed60b42e4eac821c99e19",
  measurementId: "G-1JCH91PS11",
  databaseURL: "https://robotic-arm-38d44-default-rtdb.firebaseio.com" // Add this line
};

const app = initializeApp(firebaseConfig);
export const db = getFirestore(app);
export const rtdb = getDatabase(app);