#pragma	once

#define	SINGLETON( T )									\
public:													\
	static	inline	void	create_instance()			\
	{													\
		if(!instance_ptr)								\
		{												\
			instance_ptr	= new T;					\
		}												\
	}													\
	static	inline	void	delete_instance()			\
	{													\
		if(instance_ptr)								\
		{												\
			delete instance_ptr;						\
			instance_ptr	= nullptr;					\
		}												\
	}													\
	static	inline	T*	instance()						\
	{													\
		return	instance_ptr ? instance_ptr : nullptr;	\
	}													\
	static	inline	bool	is_valid()					\
	{													\
		return	instance_ptr ? true : false;			\
	}													\
private:												\
	static	inline	T*	instance_ptr;

#define	SINGLETON_DECREATE( T )							\
public:													\
	static	inline	void	delete_instance()			\
	{													\
		if(instance_ptr)								\
		{												\
			delete instance_ptr;						\
			instance_ptr	= nullptr;					\
		}												\
	}													\
	static	inline	T*	instance()						\
	{													\
		return	instance_ptr ? instance_ptr : nullptr;	\
	}													\
	static	inline	bool	is_valid()					\
	{													\
		return	instance_ptr ? true : false;			\
	}													\
private:												\
	static	inline	T*	instance_ptr;

#define	STATIC_SINGLETON( T )		\
public:								\
	static	inline	T*	instance()	\
	{								\
		static	T	instance_data;	\
		return	&instance_data;		\
	}								\
private:
