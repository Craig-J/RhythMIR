#ifndef _CJA_SINGLETON_H_
#define _CJA_SINGLETON_H_

#include <iostream>

template<class Type>
class Singleton
{
	// The one and only instance of this class
	static Type* instance_;

	explicit Singleton()
	{
		// I have absolutely no idea what these two lines do
		//int offset = (int)(Type*)1 - (int)(Singleton<Type>*)(Type*)1;
		//instance_ = (Type*)((int)this + offset);
	}

	~Singleton(){}

	// No copying
	Singleton(const Singleton&) = delete;
	// No assignment
	Singleton& operator=(const Singleton&) = delete;

public:

	// Returns the singleton
	static Type& GetInstance()
	{
		if (!instance_)
		{
			instance_ = new Type();
		}
		return (*instance_);
	}

	// Returns a pointer to the singleton
	static Type* GetInstancePtr()
	{
		if (!instance_)
		{
			instance_ = new Type();
		}
		return instance_;
	}

	// Deletes the singleton instance
	static void DestroyInstance()
	{
		if (!instance_)
		{
			std::cerr << "WARNING: Attempting to delete a null singleton." << std::endl;
		}
		delete instance_;
		instance_ = nullptr;
	}
};

// Initialises static Type* singleton_ member.
template <typename Type>
typename Type* Singleton <Type>::singleton_ = nullptr;

#endif