using System;
using System.Runtime.CompilerServices;

namespace Hazel
{
    public struct Vector3
    {
        public float X, Y, Z;
        public Vector3(float x, float y, float z)
        {
            X = x; Y = y; Z = z; 
        }
    }
    public class Main
    {
        public float FloatVar { get; set; }
        public Main()
        {
            Console.WriteLine("Main constructor!");
            CppFunction();
            NativeLog("liujianjie", 2023);
            Vector3 vec1 = new Vector3(5, 2.5f, 1);
            //Vector3 vec2;
            NativeLogVec3(ref vec1, out Vector3 vec2);
            Console.WriteLine($"{vec2.X}, {vec2.Y}, {vec2.Z}");

            Vector3 vec3 = TestNativeLogVec3(ref vec1);
            Console.WriteLine($"{vec3.X}, {vec3.Y}, {vec3.Z}");
        }
        public void PrintMessage()
        {
            Console.WriteLine("Hello World from C#!");
        }
        public void PrintInt(int value)
        {
            Console.WriteLine($"C# says: {value}");
        }
        public void PrintInts(int value1, int value2)
        {
            Console.WriteLine($"C# says: {value1} and {value2}");
        }
        public void PrintCustomMessage(string message)
        {
            Console.WriteLine($"C# says: {message}");
        }
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void CppFunction();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void NativeLog(string text, int parameter);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void NativeLogVec3(ref Vector3 vec, out Vector3 vec2);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static Vector3 TestNativeLogVec3(ref Vector3 vec);
    }
}
