/*_____________________________________________________________________________
            The MIT License (https://opensource.org/licenses/MIT)

        Copyright (c) 2017, Jonathan D. Lettvin, All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
_____________________________________________________________________________*/

#include "gg_state.h"
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

//------------------------------------------------------------------------------
/// @brief insert either case-sensitive or both case letters into tree
///
/// Distribute characters into state tables for searching.
void
Lettvin::Table::
insert (
		char* a_chars,
		auto& a_from,
		auto& a_next,
		bool a_stop,
		bool a_nibbles)
//------------------------------------------------------------------------------
{
	auto c0{a_chars[0]};
	auto c1{a_chars[1]};
	if (a_nibbles && s_nibbles)
	{
		auto upper00{ c0     & 0x0f};
		auto upper01{(c0>>4) & 0x0f};
		auto lower10{ c1     & 0x0f};
		auto lower11{(c1>>4) & 0x0f};
		char hi[2], lo[2];
		hi[0] = upper01;
		hi[1] = lower11;
		lo[0] = upper00;
		lo[1] = lower10;
		insert (hi, a_from, a_next, a_stop, false);
		insert (lo, a_from, a_next, a_stop, false);
		return;
	}
	else
	{
		debugf (1, "INSERT %2.2x and %2.2x on plane %x\n",
				a_chars[0], a_chars[1], a_from);
		Atom& element{operator[] (a_from)[c0]};
		auto to{element.tgt ()};
		a_next = a_from;
		if (to) {
			a_from = to;
		}
		else
		{
			a_from = Table::size ();
			operator++ ();
		}
		element.tgt (a_from);
		if (c0 != c1)
		{
			operator[] (a_next)[c1].tgt (a_from);
		}
	}
} // insert

//------------------------------------------------------------------------------
/// @brief insert strings into tables
void
Lettvin::Table::
insert (string_view a_str, i24_t id, size_t setindex)
{
	auto from      {s_root};
	auto next      {from};
	char last[2]   {0,0};

	// Cardinal setindex retrieves this terminal set
	set<int32_t>& setitem{s_set[setindex]};

	// Insert initial char of str for skipping.
	if (s_caseless)
	{
		s_firsts += static_cast<char> (toupper (a_str[0]));
		s_firsts += static_cast<char> (tolower (a_str[0]));
	}
	else
	{
		s_firsts += a_str[0];
	}

	// Insert a_str into state transition tree
	for (size_t I=a_str.size () - 1, i=0; i <= I; ++i)
		//for (char u:str)
	{
		char u{a_str[i]};
		bool stop{i==I};
		if (s_caseless)
		{
			last[0] = static_cast<char> (toupper (u));
			last[1] = static_cast<char> (tolower (u));
			insert (last, from, next, stop, s_nibbles);
		}
		else
		{
			last[0] = last[1] = u;
			insert (last, from, next, stop, s_nibbles);
		}
	}
	if (s_caseless)
	{
#if SETINDIRECT
		setitem.insert (id);
		for (auto c:last) operator[] (next)[c&s_mask].str (setindex);
#else
		for (auto c:last) operator[] (next)[c&s_mask].str (id);
#endif
	}
	else
	{
		debugf (1, "LINK %x %lx %x\n", next, last[0]&s_mask, id);
#if SETINDIRECT
		setitem.insert (id);
		operator[] (next)[last[0]&s_mask].str (setindex);
#else
		operator[] (next)[last[0]&s_mask].str (id);
#endif
	}
}

//------------------------------------------------------------------------------
// @brief dump tree to file
//
// TODO must output state size and table size
// TODO best practice read entire, then swap if needed
// TODO output s_order.u64 to file to establish file order
void Lettvin::Table::dump (const char* a_filename, const char* a_title)
{
	int32_t fd = open (
			a_filename,
			O_RDWR|O_CREAT,
			S_IRWXU|S_IRWXG|S_IRWXO);
	if (fd > 0)
	{
		debugf (1, "dump PASS: %s: %s\n", a_filename, a_title);
		char   zero{0};
		size_t endian{0x3736353433323130};
		const char* cdcz{"\x04\x1a"};
		size_t consumed{34+strlen (a_title)};
		size_t needed{256-consumed};

		write (fd, "gg dump:", 8);
		write (fd, "endian: ", 8);
		write (fd, &endian   , 8);
		write (fd, " title: ", 8);
		write (fd, a_title   , strlen (a_title));
		write (fd, cdcz      , 2);
		for (size_t i=0; i<needed; ++i)
		{
			write (fd, &zero, 1);
		}
		
		for (auto& state:m_table)
		{
			for (auto& atom: state.handle ())
			{
				union { unsigned integral; uint8_t u08[4]; } datum{
					.integral = atom.integral ()};
				//write (fd, &datum.u08[s_order.u08.array[0]], 1);
				//write (fd, &datum.u08[s_order.u08.array[1]], 1);
				//write (fd, &datum.u08[s_order.u08.array[2]], 1);
				//write (fd, &datum.u08[s_order.u08.array[3]], 1);
				write (fd, &datum.u08[0], 1);
				write (fd, &datum.u08[1], 1);
				write (fd, &datum.u08[2], 1);
				write (fd, &datum.u08[3], 1);
			}
		}
	}
	else
	{
		debugf (1, "dump FAIL: %s: %s\n", a_filename, a_title);
	}
	close (fd);
}

//------------------------------------------------------------------------------
// @brief load tree from file
void Lettvin::Table::load (const char* a_filename)
{
	int32_t fd = open (a_filename, O_RDONLY, 0);
	if (fd >= 0)
	{
	}
	close (fd);
}

#ifndef SEARCH_MOVED
//------------------------------------------------------------------------------
/// @brief find and report found strings
///
/// when reject list is empty, terminate on completion of accept list
/// when reject list is non-empty, terminate on first reject
void
Lettvin::Table::
follow (void* a_pointer, auto a_bytecount, const char* a_label)
//------------------------------------------------------------------------------
{
	//debugf (1, "SEARCH %s\n", a_label);
	set<i24_t> accepted  {0};
	set<i24_t> rejected  {};
	string_view contents (static_cast<char*> (a_pointer), a_bytecount);
	size_t begin = contents.find_first_of (s_firsts);
	bool done{false};
	// outer loop (skip optimization)
	while (begin != string_view::npos && !done)
	{
		//debugf (1, "ANCHOR\n");
		contents.remove_prefix (begin);
		auto tgt{1}; // State
		auto str{0}; // 
		// inner loop (Finite State Machine optimization)
		for (char c: contents)
		{
			//debugf (1, "OFFSET\n");
			auto n00{c};
			if (s_nibbles)
			{
				// Two-step for nibbles
				n00 = (c>>4) & 0xf;
				//debugf (1, "NIBBLE H %2.2x %2.2x\n", n00, tgt);
				auto element{operator[] (tgt)[n00]};
				tgt = element.tgt ();
				n00 = c & 0xf;
				//debugf (1, "NIBBLE L %2.2x %2.2x\n", n00, tgt);
				if (!tgt) break;
			}
			else
			{
				//debugf (1, "BYTE %c\n", c);
			}
			auto element = operator[] (tgt)[n00];
			tgt = element.tgt ();
			str = element.str ();
			if (str) {
#if SETINDIRECT
				set<int32_t>& setitem{s_set[str]};
				for (auto item:setitem)
				{
					if (item < 0) return; ///< Immediate rejection
					// If not immediate rejection, add to rejected list
					auto& chose{(item>0)?accepted:rejected};
					chose.insert (item);
					bool full_accept{s_accept.size () == accepted.size ()};
					// completion optimization
					done = (s_noreject && full_accept);
					if (done) break;
				}
#else
				if (str < 0) return; ///< Immediate rejection
				// If not immediate rejection, add to rejected list
				auto& chose{(str>0)?accepted:rejected};
				chose.insert (str);
				bool full_accept{s_accept.size () == accepted.size ()};
				// completion optimization
				done = (s_noreject && full_accept);
#endif
			}
			if (done || !tgt) break;
		}
		contents.remove_prefix (1);
		begin = contents.find_first_of (s_firsts);
	}

	// Report files having all accepteds and no rejecteds.
	if (!rejected.size () && accepted.size () == s_accept.size ())
	{
		auto report{fmt::format ("{}\n", a_label)};
		// Using the unix write primitive guarantees atomicity
		// This is needed to avoid thread contention
		auto wrote = write (1, report.c_str (), report.size ());
		// This next line should never be executed.
		if (wrote == -1) printf ("%s\n", a_label);
	}
} // follow
#endif // SEARCH_MOVED

