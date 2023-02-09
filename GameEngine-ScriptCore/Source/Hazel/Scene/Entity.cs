using System;
using System.Runtime.CompilerServices;

namespace Hazel
{
    public class Entity
    {
        public readonly ulong ID;   // 实体的UUID
        protected Entity() { Console.WriteLine("Entity()"); ID = 0; }
        internal Entity(ulong id)
        {
            Console.WriteLine("Entity(ulong id)"); ID = id; }// C++通过构造函数传入实体的UUID

        public Vector3 Translation {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
                return result;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            } 
        }
    }
}
