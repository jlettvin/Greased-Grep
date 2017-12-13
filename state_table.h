#pragma once

#include <vector>
#include <string_view>

namespace Lettvin
{
	using namespace std;

	typedef unsigned      integral_t;
	static_assert (sizeof(integral_t) == 4);
	typedef int32_t       i24_t;
	static_assert (sizeof(int32_t) == 4);

#if false
	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition element.
	///
	/// This class will contain the compiled and optimized state tables.
	//__________________________________________________________________________
	template<typename T=unsigned, size_t S=256>
	class
	FSM
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief FSM ctor
		FSM (size_t state_count)
			: m_count (state_count)
		{
		}

		//----------------------------------------------------------------------
		/// @brief FSM dtor
		~FSM ()
		{
			if (nullptr != m_element)
			{
				delete [] m_element;
			}
		}

		//----------------------------------------------------------------------
		/// @brief FSM ftor
		void operator ()(
				size_t  a_state,
				size_t  a_element,
				uint8_t a_target,
				size_t  a_symbol)
		{
			if (nullptr == m_element)
			{
				// allocate
				m_element = new state_t[m_count];
			}
			size_t index = (a_state * S) + a_element;
			auto& element{m_element [index]};
			element.named.target = a_target;
			element.named.symbol = a_symbol;
		}

	//------
	private:
	//------
		static const size_t bits{8*sizeof(T)};
		typedef union {
			T integral;
			struct
			{
				uint8_t target ;
				T       symbol : bits;
			} named;
		} element_t, *element_p;
		typedef element_t state_t[S];

		element_p m_element = nullptr;
		size_t m_count;
	};

#endif

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition element.
	///
	/// constructor/getters/setters for atomic element unit.
	//__________________________________________________________________________
	class
	Atom
	{
	//------
	public:
	//------
		Atom ();
		integral_t  integral ()  const;
		uint8_t     tgt ()       const;
		i24_t       str ()       const;
		void        tgt (uint8_t a_tgt);
		void        str (i24_t a_str);
	//------
	private:
	//------
		union {
			integral_t integral;                         ///< unused name
			struct { i24_t str:24; uint8_t tgt; } state; ///< strtern/state ids
		}
		m_the
		{
			.integral=0
		};
	}; // class Atom

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief Single state-transition plane of 256 Atoms
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	State
	{
	//------
	public:
	//------
		State ();
		Atom& operator[] (uint8_t a_off);
		vector<Atom>& handle ();
	//------
	private:
	//------
		vector<Atom> m_handle;
	}; // class State

	//CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
	/// @brief vector of state-transition planes sufficient to enable search
	///
	/// constructor/indexer
	//__________________________________________________________________________
	class
	Table
	{
	//------
	public:
	//------

		//----------------------------------------------------------------------
		/// @brief Table ctor (reserve many, instance 2)
		Table ();

		//----------------------------------------------------------------------
		/// @brief indexer
		State& operator[] (uint8_t a_offset);

		//----------------------------------------------------------------------
		/// @brief add State planes to vector
		void operator++ ();

		//----------------------------------------------------------------------
		/// @brief return current size of vector
		size_t size ();

		//----------------------------------------------------------------------
		/// @brief debug utility for displaying the entire table
		ostream& show_tables (ostream& a_os);

		//----------------------------------------------------------------------
		/// @brief insert strings into tables
		void insert (string_view a_str, i24_t id, size_t setindex);

		//----------------------------------------------------------------------
		/// @brief find and report found strings
		///
		/// when reject list is empty, terminate on completion of accept list
		/// when reject list is non-empty, terminate on first reject
		void search (void* a_pointer, auto a_bytecount, const char* a_label="");

		//----------------------------------------------------------------------
		/// @brief dump tree to file
		void dump (const char* filename, const char* a_title="");

		//----------------------------------------------------------------------
		/// @brief load tree from file
		void load (const char* filename);

	//--------
	protected:
	//--------

		//dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
		vector<State> m_table;                       ///< State tables

	//------
	private:
	//------

		//----------------------------------------------------------------------
		/// @brief insert either case-sensitive or both case letters into tree
		///
		/// Distribute characters into state tables for searching.
		void insert (
				char* a_chars,
				auto& a_from,
				auto& a_next,
				bool a_stop=false,
				bool a_nibbles=false);

	}; // class Table

} // namespace Lettvin
