// driver.cxx
//

#include <memory>   // std::auto_ptr
#include <iostream>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/sqlite/database.hxx>

#include "person.hxx"
#include "person_odb.h"

using namespace std;
using namespace odb::core;

int main (int argc, char* argv[]) {
  try
  {
    auto_ptr<database> db (new odb::sqlite::database ("test.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

    connection_ptr c (db->connection ());
    c->execute ("PRAGMA foreign_keys=OFF");

    int rowCount = c->execute("SELECT * FROM main.sqlite_master WHERE name LIKE '%person%' AND type='table';");
    if( rowCount < 1 ){
      odb::transaction t (db->begin());

      t.tracer(stderr_tracer);
      odb::schema_catalog::create_schema(*db, "v1");
      t.commit();
    }

    c->execute ("PRAGMA foreign_keys=ON");

    unsigned long john_id, jane_id, joe_id;

    // Create a few persistent person objects.
    //
    {
      person john ("John", "Doe", 33);
      person jane ("Jane", "Doe", 32);
      person joe ("Joe", "Dirt", 30);

      transaction t (db->begin ());

      // Make objects persistent and save their ids for later use.
      //
      john_id = db->persist (john);
      jane_id = db->persist (jane);
      joe_id = db->persist (joe);

      t.commit ();
    }

    // Joe Dirt just had a birthday, so update his age.
    //
    {
      transaction t (db->begin ());

      auto_ptr<person> joe (db->load<person> (joe_id));
      joe->setAge (joe->getAge () + 1);
      db->update (*joe);

      t.commit ();
    }
  }
  catch (const odb::exception& e)
  {
    cerr << e.what () << endl;
    return 1;
  }
}
