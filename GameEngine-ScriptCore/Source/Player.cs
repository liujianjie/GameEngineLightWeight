using System;
using Hazel;

namespace Sandbox
{
    public class Player : Entity
    {
        public Player()
        {
            Console.WriteLine("Player()");
        }
        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate() - {ID}");
        }
        void OnUpdate(float ts)
        {
            //Console.WriteLine($"Player.OnUpdate() - {ts}");
            float speed = 1.0f;
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
            Vector3 translation = Translation; // get是调用C++的内部函数获取实体的位置
            translation += velocity * ts;
            Translation = translation;          // set是调用C++的内部函数设置实体的位置
        }
    }
}
