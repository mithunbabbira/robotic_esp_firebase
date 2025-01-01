import LoginForm from "@/components/LoginForm";

const Index = () => {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center p-4 bg-gray-50">
      <div className="text-center mb-8">
        <h1 className="text-3xl font-bold mb-2">Robotic Arm Control Panel</h1>
        <p className="text-gray-600">Please login to access the controller</p>
      </div>
      <LoginForm />
    </div>
  );
};

export default Index;