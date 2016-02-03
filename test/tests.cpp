//   -----------------------------------------------------------------------------------------------
//    Copyright 2015 André Bergner. Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//      --------------------------------------------------------------------------------------------

#include <boost/core/lightweight_test.hpp>

#include <flowz/flowz.hpp>


//  ------------------------------------------------------------------------------------------------
// testing internal transforms
//  ------------------------------------------------------------------------------------------------

void test_unary_to_binary()
{
   using namespace flowz;

   transforms::unary_to_binary_feedback  un2bin;

   auto is_same = []( auto x, auto y )
   {  return  std::is_same< decltype(x) , decltype(y) >::value;  };

   auto bin_fb = []( auto x, auto y)
   {  return make_binary_feedback(x,y);  };

   BOOST_TEST(is_same(  bin_fb( _1 , _1[_1] ) , un2bin(~( _1 |= _1[_1] ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , _1[_1] ) , un2bin(~( (_1 |= _1) |= _1[_1] ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , _1[_1] ) , un2bin(~( _1 |= (_1 |= _1[_1]) ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 ,  _1[_1] |= (_1 |= _1[_1]) ) , un2bin(~( (_1  |=  _1[_1]) |= (_1  |=  _1[_1]) ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 , (_1[_1] |= _1) |= _1[_1]  ) , un2bin(~( (_1  |=  _1[_1]  |=  _1) |= (_1[_1]) ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 , _1[_1] |= _1 |= _1[_1] ) , un2bin(~( (_1) |= (_1[_1]  |=  _1  |=  _1[_1]) ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 , _1[_1] +_2 ) , un2bin(~( _1 |= _1[_1] + _2 ))  ));
   BOOST_TEST(is_same(  bin_fb( _1+2 , _1[_1] + _2 ) , un2bin(~( _1+2 |= _1[_1] + _2 ))  ));
   BOOST_TEST(is_same(  bin_fb( _1+2 , _1[_1] - 13 + _2 ) , un2bin(~( _1+2 |= _1[_1] - 13 + _2 ))  ));

   BOOST_TEST(is_same(  bin_fb( _2 , _1[_1] + _2[_1] ) , un2bin(~( _2 |= _1[_1] + _2[_1] ))  ));
   BOOST_TEST(is_same(  bin_fb( _2 |= _1 , _1[_1] + _2[_1] ) , un2bin(~( _2 |= _1 |= _1[_1] + _2[_1] ))  ));
}



void test_wires_around_boxes()
{
   using namespace flowz;
   input_arity   ins;
   output_arity  outs;

   auto wire_around_prev_box = ( _1 |= _2 );
   BOOST_TEST_EQ( 2 , ins(wire_around_prev_box) );
   BOOST_TEST_EQ( 1 , outs(wire_around_prev_box) );
   auto wp = compile( wire_around_prev_box );
   BOOST_TEST( std::make_tuple(1337) == wp(2,1337) );

   auto wire_around_succ_box = ( (_1,_1) |= _1);
   BOOST_TEST_EQ( 1 , ins(wire_around_succ_box) );
   BOOST_TEST_EQ( 2 , outs(wire_around_succ_box) );
   auto ws = compile( wire_around_succ_box );
   BOOST_TEST( std::make_tuple(1337,1337) == ws(1337) );
}


void test_simple_expressions()
{
   using namespace flowz;

   auto identity = compile( _1 );

   BOOST_TEST( std::make_tuple(1337) == identity(1337) );
   BOOST_TEST( std::make_tuple(42)   == identity(42) );


   auto unit_delay = compile( _1[_1] );

   BOOST_TEST( std::make_tuple(0)    == unit_delay(1337) );
   BOOST_TEST( std::make_tuple(1337) == unit_delay(42) );
   BOOST_TEST( std::make_tuple(42)   == unit_delay(17) );


   auto differntiator = compile( _1 - _1[_1] );

   BOOST_TEST( std::make_tuple(1337)    == differntiator(1337) );
   BOOST_TEST( std::make_tuple(42-1337) == differntiator(42) );
   BOOST_TEST( std::make_tuple(17-42)   == differntiator(17) );


   auto integrator = compile( ~(_1[_1] + _2) );

   BOOST_TEST( std::make_tuple(1337)       == integrator(1337) );
   BOOST_TEST( std::make_tuple(1337+42)    == integrator(42) );
   BOOST_TEST( std::make_tuple(1337+42+17) == integrator(17) );

}



int main()
{
   test_unary_to_binary();
   test_wires_around_boxes();
   test_simple_expressions();

   return boost::report_errors();
}