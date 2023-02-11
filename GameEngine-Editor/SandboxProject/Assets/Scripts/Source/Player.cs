using System;
using Hazel;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;

        public Player()
        {
            Console.WriteLine("Player()");
            //Console.WriteLine("typeof(Player){0}", typeof(Player));
            
        }
        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate() - {ID}");
            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }
        void OnUpdate(float ts)
        {
            //Console.WriteLine($"Player.OnUpdate() - {ts}");
            float speed = 0.2f;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.Y = 1.0f;
                Console.WriteLine("press the W key");
            }
            else if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.Y = -1.0f;
                Console.WriteLine("press the S key");
            }
            else if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.X = -1.0f;
                Console.WriteLine("press the A key");
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.X = 1.0f;
                Console.WriteLine("press the D key");
            }
            velocity *= speed;
            //Console.WriteLine($"velocity={velocity.X},{velocity.Y}");
            // 121节
            // Rigidbody2DComponent 控制物体移动
            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

            // 普通Transform组件
            //Vector3 translation = m_Transform.Translation; // get是调用C++的内部函数获取实体的位置
            //translation += velocity * ts;
            //m_Transform.Translation = translation;          // set是调用C++的内部函数设置实体的位置
            // 120节
            //Vector3 translation = Translation; // get是调用C++的内部函数获取实体的位置
            //translation += velocity * ts;
            //Translation = translation;          // set是调用C++的内部函数设置实体的位置
        }
    }

}
