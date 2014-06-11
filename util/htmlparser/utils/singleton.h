#ifndef _SINGLETON_H_
#define _SINGLETON_H_

template<class T>
class CSingleton {
 public:
  static T& Instance();

 private:
  static T m_oInstance;
 private:
  CSingleton() {}
  CSingleton(const CSingleton&);
  CSingleton& operator=(const CSingleton&);
};

template<class T> T CSingleton<T>::m_oInstance;

template<class T>
T& CSingleton<T>::Instance() {
  return m_oInstance;
}

#endif

