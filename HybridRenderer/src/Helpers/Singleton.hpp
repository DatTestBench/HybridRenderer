#ifndef SINGLETON_HPP
#define SINGLETON_HPP
template <typename T>
class Singleton
{
public:
    static T* GetInstance()
    {
        if (m_pInstance == nullptr)
            m_pInstance = new T(Token());
        return m_pInstance;
    }
    static void Destroy()
    {
        SafeDelete(GetInstance());
    }
    DEL_ROF(Singleton)

protected:
    static T* m_pInstance;
    struct Token {};
    Singleton() = default;
};

template <typename T>
T* Singleton<T>::m_pInstance = nullptr;

#endif // !SINGLETON_HPP