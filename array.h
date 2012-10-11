#ifndef ARRAY_H
#define ARRAY_H

#include <cstdlib>
#include <iostream>

//#define ARRAY_DEBUG

#define ARRAY_DEFAULT_CAPACITY 100
#define ARRAY_CAPACITY_STEP 50
#define ERR_INDEX_OUT_OF_BOUNDS 1
#define ERR_OUT_OF_MEMORY 2

/**
 * \todo implement implicit sharing for the array of elements
 */
template<class T> class Array {
	public:
		Array( int capacity = ARRAY_DEFAULT_CAPACITY )
		:	m_elements(0),
			m_count(0),
			m_capacity(0),
			m_refcount(0) {
//std::cerr << "Array::[constructor]()\n";
			m_refcount = new int(1);
			if(capacity < 0) capacity = ARRAY_DEFAULT_CAPACITY;
			reserve(capacity);
#if defined(ARRAY_DEBUG)
			m_objectIndex = s_nextObjectIndex++;
			std::cerr << "created Array #" << m_objectIndex << ", refCount = " << (*m_refcount) << "\n";
#endif
		}

		Array( const Array<T> & other )
		:	m_elements(other.m_elements),
			m_count(other.m_count),
			m_capacity(other.m_capacity),
			m_refcount(other.m_refcount) {
//std::cerr << "Array::[copyConstructor]()\n";
			++(*m_refcount);
#if defined(ARRAY_DEBUG)
			m_objectIndex = s_nextObjectIndex++;
			std::cerr << "created Array #" << m_objectIndex << ", refCount = " << (*m_refcount) << "\n";
#endif
		}

		Array<T> & operator=( const Array<T> & other ) {
			dispose();

			m_elements = other.m_elements;
			m_count = other.m_count;
			m_capacity = other.m_capacity;
			m_refcount = other.m_refcount;
			++(*m_refcount);
			return *this;
		}

		virtual ~Array( void ) {
//std::cerr << "Array::~Array()\n";
			dispose();
		}

		void dispose( void ) {
//std::cerr << "Array::dispose()\n";
#if defined(ARRAY_DEBUG)
			std::cerr << "disposing of Array #" << m_objectIndex << ", refCount = " << (*m_refcount) << "\n";
#endif
			if(0 == --(*m_refcount)) {
std::cerr << "refcount now " << (*m_refcount) << ": deleting data.\n";
				if(m_elements) {
					clear();
					std::free(m_elements);
					m_elements = 0;
					m_capacity = 0;
					delete m_refcount;
					m_refcount = 0;
				}
			}
		}

		inline int capacity( void ) const {
			return m_capacity;
		}

		inline int size( void ) const {
			return count();
		}

		inline int count( void ) const {
			return m_count;
		}

		inline bool isEmpty( void ) const {
			return 0 == m_count;
		}

		bool append( const T& item ) {
//std::cerr << "Array::append()\n";
			deepCopy();

			if(m_count >= m_capacity && !reserve(m_capacity + ARRAY_CAPACITY_STEP))
				return false;

			m_elements[m_count++] = new T(item);
			return true;
		}

		inline Array<T> & operator<<( const T& item ) {
//std::cerr << "Array::operator<<()\n";
			append(item);
			return *this;
		}

		void removeFirst( const T& item ) {
//std::cerr << "Array::removeFirst()\n";
			for(int i = 0; i < m_count; ++i) {
				if(item == *(m_elements[i])) {
					deepCopy();
					delete m_elements[i];

					for(int j = i + 1; j < m_count; ++j)
						m_elements[j - 1] = m_elements[j];

					break;
				}
			}
		}

		void removeAll( const T& item ) {
//std::cerr << "Array::removeAll()\n";
			bool doneDeepCopy = false;

			for(int i = 0; i < m_count; ++i) {
				if(item == *(m_elements[i])) {
					if(!doneDeepCopy) {
						deepCopy();
						doneDeepCopy = true;
					}

					delete m_elements[i];

					for(int j = i + 1; j < m_count; ++j)
						m_elements[j - 1] = m_elements[j];

					--	m_count;
					--i;
				}
			}
		}

		void clear( void ) {
//std::cerr << "Array::clear()\n";
			if(0 == m_count) return;

			/* calling deepCopy() is sub-optimal as it copies all elements that we
			 * are immediately going to discard, so we implement our own deref
			 * here that doesn't actually copy anything. */

			if(2 > *m_refcount) {
				/* if we're not sharing data with any other object, delete all the
				 * elements */
				for(int i = 0; i < m_count; ++i)
					delete m_elements[i];
			}
			else {
				/* otherwise, do everything a deep copy would do, except copy the
				 * actual elements, which is a waste of time because they're all
				 * going to be deleted immediately */
				T ** newElements = (T **) std::malloc((sizeof(T *) * m_capacity));
				if(!newElements) return throw ERR_OUT_OF_MEMORY;
				--(*m_refcount);
				m_refcount = new int(1);
				m_elements = (T **) newElements;
			}

			m_count = 0;
		}

		inline const T & item( int i ) const {
//std::cerr << "Array::item()\n";
			if(i < 0 || i >= count()) throw ERR_INDEX_OUT_OF_BOUNDS;
			return  *(m_elements[i]);
		}

		inline const T & operator[]( int i ) const {
//std::cerr << "Array::operator[]()\n";
			return item(i);
		}

		inline T & operator[]( int i ) {
//std::cerr << "Array::operator[]()\n";
			if(i < 0 || i >= count()) throw ERR_INDEX_OUT_OF_BOUNDS;
			deepCopy();
			return *(m_elements[i]);
		}

		bool reserve( int i ) {
//std::cerr << "Array::reserve()\n";
			if(i > m_capacity) {
				deepCopy();
				void * newElements = std::realloc(m_elements, (sizeof(T *) * i));
				if(!newElements) return false;
				m_elements = (T **) newElements;
				m_capacity = i;
			}

			return true;
		}

		bool contract( void ) {
//std::cerr << "Array::contract()\n";
			if(m_count < m_capacity) {
				deepCopy();
				void * newElements = std::realloc(m_elements, (sizeof(T *) * m_count));
				if(!newElements) return false;
				m_elements = (T **) newElements;
				m_capacity = m_count;
			}
		}

//		void copy( const Array<T> & other ) {
//			clear();
//			if(other.m_capacity > m_capacity && !reserve(other.m_capacity)) throw ERR_OUT_OF_MEMORY;

//			for(int i = 0; i < other.m_count; ++i)
//				m_elements[i] = new T(*(other.m_elements[i]));

//			m_capacity = other.m_capacity;
//			m_count = other.m_count;
//		}

		void deepCopy( void ) {
			/* NOTE this condition must handle cases where refcount is 1 and 0 even
			 * though refcount should never drop below 0. this is because the dispose()
			 * method decrements refcount before it calls clear(), and as clear() calls
			 * deepCopy(), refcount can be 0 when we reach here. */
			if(2 > *m_refcount) {
//std::cerr << "refcount = " << (*m_refcount) << ": no need to take a deep copy.\n";
				return;
			}

//std::cerr << "refcount = " << (*m_refcount) << ": taking a deep copy.\n";
			T ** newElements = (T **) std::malloc((sizeof(T *) * m_capacity));
			if(!newElements) return throw ERR_OUT_OF_MEMORY;

			for(int i = 0; i < m_count; ++i)
				newElements[i] = new T(*(m_elements[i]));

			--(*m_refcount);
			m_refcount = new int(1);
			m_elements = (T **) newElements;
		}

	private:
		T ** m_elements;
		int m_count, m_capacity;
		int * m_refcount;

#if(defined(ARRAY_DEBUG))
		int m_objectIndex;
		static int s_nextObjectIndex;
#endif
};

#if(defined(ARRAY_DEBUG))
template <class T> int Array<T>::s_nextObjectIndex = 0;
#endif


template<class T> void operator<<( const std::iostream & out, const Array<T> & a ) {
	out << "\nArray\n-----\nSize = " << a.count();
	out << "\nCapacity = " << a.capacity();

	for(int i = 0; i < a.count(); ++i) {
		if(!(i % 10)) out << "\n";
		out << a[i] << ", ";
	}

	out << "\n";
}

#endif // ARRAY_H
