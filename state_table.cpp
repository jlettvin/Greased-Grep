//#define STATE_TABLE_CPP

#ifdef STATE_TABLE_CPP
#include "state_table.h"
#include "gg.h"



//------------------------------------------------------------------------------
Lettvin::Atom::
Atom ()
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
Lettvin::integral_t
Lettvin::Atom::
integral () const
{
	return m_the.integral;
} // integral

//------------------------------------------------------------------------------
uint8_t
Lettvin::Atom::
tgt () const
//------------------------------------------------------------------------------
{
	return m_the.state.tgt;
} // tgt

//------------------------------------------------------------------------------
Lettvin::i24_t
Lettvin::Atom::
str () const
//------------------------------------------------------------------------------
{
	return m_the.state.str;
} // str

//------------------------------------------------------------------------------
void
Lettvin::Atom::
tgt (uint8_t a_tgt)
//------------------------------------------------------------------------------
{
	m_the.state.tgt = a_tgt;
} // tgt

//------------------------------------------------------------------------------
void
Lettvin::Atom::
str (i24_t a_str)
//------------------------------------------------------------------------------
{
	m_the.state.str = a_str;
} // str

//SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS

//------------------------------------------------------------------------------
Lettvin::State::
State ()
//------------------------------------------------------------------------------
	: m_handle (s_size)
{
} // State

//------------------------------------------------------------------------------
Lettvin::Atom&
Lettvin::State::
operator[] (uint8_t a_off)
//------------------------------------------------------------------------------
{
	return m_handle[a_off];
} ///< operator[]

//------------------------------------------------------------------------------
std::vector<Lettvin::Atom>&
Lettvin::State::
handle ()
//------------------------------------------------------------------------------
{
	return m_handle;
}

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

//------------------------------------------------------------------------------
Lettvin::Table::
Table ()
//------------------------------------------------------------------------------
{
	m_table.reserve (s_prefill*256);
	for (size_t i=0; i < s_prefill; ++i)
	{
		operator++ ();
		operator++ ();
	}
} // ctor

//------------------------------------------------------------------------------
Lettvin::State&
Lettvin::Table::
operator[] (uint8_t a_offset)
//------------------------------------------------------------------------------
{
	return m_table[a_offset & s_mask];
} // operator[]

//------------------------------------------------------------------------------
void
Lettvin::Table::
operator++ ()
//------------------------------------------------------------------------------
{
	m_table.resize (m_table.size () + 1);
} // operator++

//------------------------------------------------------------------------------
size_t
Lettvin::Table::
size ()
//------------------------------------------------------------------------------
{
	return m_table.size ();
} // size

//------------------------------------------------------------------------------
std::ostream&
Lettvin::Table::
show_tables (ostream& a_os)
//------------------------------------------------------------------------------
{
	size_t COLS{s_nibbles ? 4ULL : 16ULL};
	size_t ROWS{s_nibbles ? 4ULL : 16ULL};
	for (size_t state=0; state < m_table.size (); ++state)
	{
		auto& plane{m_table[state]};
		a_os << " # " << endl << " # ";
		for (unsigned col=0; col < COLS; ++col)
		{
			printf ("   %2.2x", col);
		}
		a_os << endl << " #  " << string (5*COLS, '_') << 
			"     PLANE: " << state << endl << " # ";
		for (unsigned row=0; row < ROWS*COLS; row+=COLS)
		{
			bool content{false};
			for (unsigned col=0; col < COLS; ++col)
			{
				Atom& entry{plane[static_cast<char>(row+col)]};
				int32_t tgt{static_cast<int32_t>(entry.tgt ())};
				int32_t str{static_cast<int32_t>(entry.str ())};
				content |= !!tgt;
				content |= !!str;
			}
			if (!content)
			{
				continue;
			}
			a_os << '|';

			for (unsigned col=0; col < COLS; ++col)
			{
				char id{static_cast<char>(row+col)};
				Atom& entry{plane[id]};
				int32_t tgt{static_cast<int32_t>(entry.tgt ())};
				char gra{id>=' '&&id<='~'?id:'?'};
				if (tgt) a_os << gra << setw (3) << tgt << ' ';
				else     a_os << ".....";
			}
			printf ("|\n # |");
			for (size_t col=0; col < COLS; ++col)
			{
				Atom& entry{plane[row+col]};
				int32_t str{static_cast<int32_t>(entry.str ())};
				if (str) a_os << setw (4) << str << ' ';
				else     a_os << ".....";
			}
			printf ("|%2.2x\n # ", row);
		}
		a_os << "|" << string (5*COLS, '_') << "|" << endl;
	}
	return a_os;
} // show_tables

#endif //STATE_TABLE_CPP
