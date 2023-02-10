#pragma once
namespace Hazel {
	class ScriptGlue
	{
	public:
		// 添加内部调用
		static void RegisterFunctions();
		static void RegisterComponents();
	};
}

