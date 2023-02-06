#pragma once

namespace Hazel {
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std {
	template<typename T> struct hash;// 不清楚这个干嘛用的

	template<>
	struct hash<Hazel::UUID> {
		std::size_t operator()(const Hazel::UUID& uuid) const {
			//return hash<uint64_t>()((uint64_t)uuid);
			return (uint64_t)uuid;
		}
	};
}

