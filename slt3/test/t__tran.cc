/*
Copyright (c) 2008,2009, David Beck

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
   @file t__tran.cc
   @brief Tests to check and measure various slt3::tran features
 */

#include "common.h"
#include "test_timer.h"
#include "tran.hh"
#include "conn.hh"
#include "synqry.hh"
#include "exc.hh"
#include "str.hh"
#include "ustr.hh"
#include <assert.h>

using namespace csl::slt3;
using csl::common::str;
using csl::common::ustr;

/** @brief contains tests related to slt3::tran */
namespace test_tran {

  /** @test baseline for performance comparison */
  void baseline()
  {
    conn c;
    c.use_exc(false);
    tran t(c);
    assert( c.use_exc() == false );
    assert( t.use_exc() == false );
  }

  /** @test commit */
  void test_commit()
  {
    conn c;
    c.use_exc(false);
    assert(c.open("test.db") == true );
    {
      tran t(c);
      synqry q(t);
      assert( q.execute("CREATE TABLE t(i INT);") == true );
      assert( q.execute("INSERT INTO t VALUES(1);") == true );
      t.commit();
    }
    assert( c.close() == true );
    assert(c.open("test.db") == true );
    {
      tran t(c);
      synqry q(t);
      ustr s;
      assert( q.execute("SELECT SUM(i) FROM T;",s) == true );
      assert( s == "1" );
      assert( q.execute("DROP TABLE T;",s) == true );
      t.commit();
    }
  }

  /** @test commit on destruct */
  void commit_on_destr()
  {
    conn c;
    c.use_exc(false);
    assert(c.open("test.db") == true );
    {
      tran t(c);
      t.commit_on_destruct(true);
      synqry q(t);
      assert( q.execute("CREATE TABLE t(i INT);") == true );
      assert( q.execute("INSERT INTO t VALUES(1);") == true );
    }
    {
      tran t(c);
      t.commit_on_destruct(false);
      synqry q(t);
      assert( q.execute("INSERT INTO t VALUES(3);") == true );
    }
    assert( c.close() == true );
    assert(c.open("test.db") == true );
    {
      tran t(c);
      t.commit_on_destruct(true);
      synqry q(t);
      ustr s;
      assert( q.execute("SELECT SUM(i) FROM T;",s) == true );
      assert( s == "1" );
      assert( q.execute("DROP TABLE T;",s) == true );
    }
  }

  /** @test rollback */
  void test_rollback()
  {
    conn c;
    c.use_exc(true);
    bool caught = false;
    assert(c.open("test.db") == true );
    {
      tran t(c);
      synqry q(t);
      assert( q.execute("CREATE TABLE t(i INT);") == true );
      assert( q.execute("INSERT INTO t VALUES(1);") == true );
      t.rollback();
    }
    assert( c.close() == true );
    assert( c.open("test.db") == true );
    try
    {
      tran t(c);
      synqry q(t);
      ustr s;
      assert( q.execute("SELECT SUM(i) FROM T;",s) == true );
      assert( s == "1" );
    }
    catch( exc e )
    {
      caught = true;
    }
    assert( caught == true );
  }

  /** @test rollback on destruct */
  void rollb_on_destr()
  {
    conn c;
    c.use_exc(true);
    bool caught = false;
    assert(c.open("test.db") == true );
    {
      tran t(c);
      t.rollback_on_destruct(true);
      synqry q(t);
      assert( q.execute("CREATE TABLE t(i INT);") == true );
      assert( q.execute("INSERT INTO t VALUES(1);") == true );
    }
    assert( c.close() == true );
    assert( c.open("test.db") == true );
    try
    {
      tran t(c);
      t.rollback_on_destruct(false);
      synqry q(t);
      ustr s;
      assert( q.execute("SELECT SUM(i) FROM T;",s) == true );
      assert( s == "1" );
    }
    catch( exc e )
    {
      caught = true;
    }
    assert( caught == true );
  }

} // end of test_tran

using namespace test_tran;

int main()
{
  UNLINK( "test.db" );
  csl_common_print_results( "baseline        ", csl_common_test_timer_v0(baseline),"" );
  csl_common_print_results( "test_commit     ", csl_common_test_timer_v0(test_commit),"" );
  csl_common_print_results( "test_rollback   ", csl_common_test_timer_v0(test_rollback),"" );
  csl_common_print_results( "commit_on_destr ", csl_common_test_timer_v0(commit_on_destr),"" );
  csl_common_print_results( "rollb_on_destr  ", csl_common_test_timer_v0(rollb_on_destr),"" );
  UNLINK( "test.db" );
  return 0;
}

/* EOF */
