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

   transforms::make_canonical  un2bin;

   auto is_same = []( auto x, auto y )
   {  return  std::is_same< decltype(x) , decltype(y) >::value;  };

   auto bin_fb = []( auto x, auto y)
   {  return make_binary_feedback(x,y);  };

   BOOST_TEST(is_same(  bin_fb( _1 , _1[_1] ) , un2bin(~( _1[_1] ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , _1[_1] ) , un2bin(~( _1 |= _1[_1] ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 |= _1 |= _1 , _1[_1] ) , un2bin(~( (_1 |= _1) |= _1[_1] ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1 |= _1 , _1[_1] ) , un2bin(~( _1 |= (_1 |= _1[_1]) ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 |= _1 ,  _1[_1] |= (_1 |= _1[_1]) ) , un2bin(~( (_1  |=  _1[_1]) |= (_1  |=  _1[_1]) ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , (_1[_1] |= _1) |= _1[_1]  ) , un2bin(~( (_1  |=  _1[_1]  |=  _1) |= (_1[_1]) ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , _1[_1] |= _1 |= _1[_1] ) , un2bin(~( (_1) |= (_1[_1]  |=  _1  |=  _1[_1]) ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 |= _1 , _1[_1] +_2 ) , un2bin(~( _1 |= _1[_1] + _2 ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1+2 , _1[_1] + _2 ) , un2bin(~( _1+2 |= _1[_1] + _2 ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= _1+2 , _1[_1] - 13 + _2 ) , un2bin(~( _1+2 |= _1[_1] - 13 + _2 ))  ));

   BOOST_TEST(is_same(  bin_fb( _1|_1 |= _1 + _2 , _1[_1] |= (_1,_1) ) , un2bin(~( _1 + _2 |= _1[_1] |= (_1,_1) ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 , _2 |= _1 ) , un2bin(~( _2 |= _1 ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 , _2 |= _1[_1] + _2[_1] ) , un2bin(~( _2 |= _1[_1] + _2[_1] ))  ));
   BOOST_TEST(is_same(  bin_fb( _1 , _2 |= _1 |= _1[_1] + _2[_1] ) , un2bin(~( _2 |= _1 |= _1[_1] + _2[_1] ))  ));

   BOOST_TEST(is_same(  bin_fb( _1 |= _1 + _2 , _1[_1] ) , un2bin(~( _1 + _2 |= _1[_1] ) )  ));

   //  ---------------------------------------------------------------------------------------------
   // expressions with comma

   BOOST_TEST(is_same(  bin_fb( _1 |= (_1,_1) , _1[_1] + _2[_1] ) , un2bin(~( (_1,_1) |= _1[_1] + _2[_1] ) )  ));
   BOOST_TEST(is_same(  bin_fb( _1 |= (_1,_1) |= _1 + _2 , _1[_1] ) , un2bin(~( (_1,_1) |= _1 + _2 |= _1[_1] ) )  ));

   //  ---------------------------------------------------------------------------------------------
   // nested feedback operators

   BOOST_TEST(is_same(  bin_fb( _1 , bin_fb( _1 , _1[_1] + _2[_1] )) , un2bin(~~( _1[_1] + _2[_1] ) )  ));
   //BOOST_TEST(is_same(  bin_fb( _1 , bin_fb( _1 |= _1 + _2 , _1[_1] )) , un2bin(~~( _1 + _2 |= _1[_1] ) )  ));
   BOOST_TEST(is_same(  bin_fb( _1 , _1[_1] |= bin_fb( _1 , _1[_1] + _2 )) , un2bin(~(_1[_1] |= ~( _1[_1] + _2 )) )  ));


   input_arity       ins;
   output_arity      outs;
   max_input_delays  del;

   {  auto x = un2bin( ~( _1 + _2[_1] |= _1[_1] + _2 ) );
      BOOST_TEST_EQ( 2 , ins(x) );
      BOOST_TEST_EQ( 1 , outs(x) );
      BOOST_TEST( std::make_tuple(1,0) == del(x) );
   }

   {  auto x = un2bin( ~( _1 + _3[_1] |= _1[_1] + _2 ) );
      BOOST_TEST_EQ( 3 , ins(x) );
      BOOST_TEST_EQ( 1 , outs(x) );
      BOOST_TEST( std::make_tuple(0,1,0) == del(x) );
   }
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
   auto wpr = wp(2,1337);
   BOOST_TEST( std::make_tuple(1337) == wpr );
   BOOST_TEST_EQ( 1 , std::tuple_size<decltype(wpr)>::value );

   auto wire_around_succ_box = ( (_1,_1) |= _1);
   BOOST_TEST_EQ( 1 , ins(wire_around_succ_box) );
   BOOST_TEST_EQ( 2 , outs(wire_around_succ_box) );
   auto ws = compile( wire_around_succ_box );
   auto wsr = ws(1337);
   BOOST_TEST( std::make_tuple(1337) == wsr );
   BOOST_TEST_EQ( 2 , std::tuple_size<decltype(wsr)>::value );
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


void test_delayed_sequences()
{
   using namespace flowz;

   {  auto f = compile( _1 |= _1[_1] );
      BOOST_TEST( std::make_tuple(0)    == f(1337) );
      BOOST_TEST( std::make_tuple(1337) == f(0) );
      BOOST_TEST( std::make_tuple(0)    == f(0) );
   }

   {  auto f = compile( _1 |= (_1[_1],_2[_2]) );
      BOOST_TEST( std::make_tuple(0,0)     == f(1337,42) );
      BOOST_TEST( std::make_tuple(1337,0)  == f(0,0)     );
      BOOST_TEST( std::make_tuple(0,42)    == f(0,0)     );
   }
}



void test_feedback_expressions()
{
   using namespace flowz;

   auto integrator1 = compile( ~(_1[_1] + _2) );

   BOOST_TEST( std::make_tuple(1337)       == integrator1(1337) );
   BOOST_TEST( std::make_tuple(1337+42)    == integrator1(42) );
   BOOST_TEST( std::make_tuple(1337+42+17) == integrator1(17) );

   auto integrator2 = compile( ~(_1[_1] + _2 |= _1) );

   BOOST_TEST( std::make_tuple(1337)       == integrator2(1337) );
   BOOST_TEST( std::make_tuple(1337+42)    == integrator2(42) );
   BOOST_TEST( std::make_tuple(1337+42+17) == integrator2(17) );

   auto integrator3 = compile( ~(_1 |= _1[_1] + _2) );

   BOOST_TEST( std::make_tuple(1337)       == integrator3(1337) );
   BOOST_TEST( std::make_tuple(1337+42)    == integrator3(42) );
   BOOST_TEST( std::make_tuple(1337+42+17) == integrator3(17) );
}


#include <complex>

void test_result_type_transform()
{
   using namespace flowz;
   using namespace flowz::transforms;

   using std::tuple;
   using cplx = std::complex<float>;

   ResultType    r;
   tuple<float>  x;

   auto expect_type = [r,x]( auto expected, auto expression )
   {
      return std::is_same< decltype(expected) , decltype( r(expression,x) ) >::value;
   };

   BOOST_TEST(expect_type( float{}         , _1       ));    // float ?   should be tuple<float>
   BOOST_TEST(expect_type( double{}        , _1 * 1.0 ));    // double ?  should be tuple<double>

   BOOST_TEST(expect_type( tuple<float>{}  , _1 |= _1 ));
   BOOST_TEST(expect_type( tuple<double>{} , _1 * 1.0 |= _1 ));
   BOOST_TEST(expect_type( tuple<double>{} , _1 |= 1.0 * _1 ));

   BOOST_TEST(expect_type( tuple<cplx>{}   , _1 |= cplx{1,0} * _1 ));
   BOOST_TEST(expect_type( tuple<cplx>{}   , 2*_1 |= _1*cplx{1,0} |= _1 ));

   BOOST_TEST(expect_type( tuple<float,float>{}   , (_1,_1) ));
   BOOST_TEST(expect_type( tuple<float,double>{}  , (_1,_1*1.0) ));
   BOOST_TEST(expect_type( tuple<double,float>{}  , (_1*1.0,_1) ));
   BOOST_TEST(expect_type( tuple<double,double>{} , (1.0*_1,1.0*_1) ));
   BOOST_TEST(expect_type( tuple<float,double>{}  , (_1*1.0,_1) |= (_2,_1) ));
   BOOST_TEST(expect_type( tuple<float,double>{}  , (_1,1.0*_1) |= (_1|_1) ));

   BOOST_TEST(expect_type( float{}         , _1[_1] ));         // float ?   should be tuple<float>
   BOOST_TEST(expect_type( tuple<float>{}  , _1 |= _1[_1] ));
   BOOST_TEST(expect_type( tuple<double>{} , (_1[_1],1.0*_1) |= _2[_1] ));

   BOOST_TEST(expect_type( tuple<float>{}   , ~( _1[_1] + _2 ) ));
   BOOST_TEST(expect_type( tuple<double>{}  , ~( 1.0*_1[_1] + _2 ) ));
   BOOST_TEST(expect_type( tuple<double>{}  , ~( _1[_1] + 1.0*_2 ) ));
   BOOST_TEST(expect_type( tuple<double>{}  , ~( _1[_1] + 1.0 ) ));

   make_canonical  c;

   BOOST_TEST(expect_type( tuple<float>{}   , c( ~( _1[_1] + _2 ) )));
   BOOST_TEST(expect_type( tuple<double>{}  , c( ~( 1.0*_1[_1] + _2 ) )));
   BOOST_TEST(expect_type( tuple<double>{}  , c( ~( _1[_1] + 1.0*_2 ) )));
   BOOST_TEST(expect_type( tuple<double>{}  , c( ~( _1[_1] + 1.0 ) )));
}



int main()
{
   test_unary_to_binary();
   test_wires_around_boxes();
   test_simple_expressions();
   test_delayed_sequences();
   test_feedback_expressions();
   test_result_type_transform();

   return boost::report_errors();
}
