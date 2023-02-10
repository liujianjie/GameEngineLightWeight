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
        // C#的泛型
        public bool HasComponent<T>() where T : Component, new()// new()是确保有空构造函数
        {
            Type componentType = typeof(T);// 得到命名空间.类名名称，比如Sandbox.Player
            Console.WriteLine($"{typeof(T)}");
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
            {
                return null;
            }
            T component = new T() { Entity = this };// 返回本地类实例对象
            return component;
        }
    }
}
