//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\unittest.h"
#include "utilites\application.h"

//////////////////////////////////////////////////////////////////////////

#include "serl/archive.h"

#include "serl/serl_text.h"
#include "serl/serl_data.h"
#include "serl/serl_registry.h"

#ifdef _WIN64
#include "cryptopp70/cryptlib.h"
#include "cryptopp70/base64.h"
#include "cryptopp70/gzip.h"
#include "cryptopp70/hex.h"
#else
#include "cryptopp/cryptlib.h"
#include "cryptopp/base64.h"
#include "cryptopp/gzip.h"
#include "cryptopp/hex.h"
#endif

using namespace CryptoPP;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( serl_test )
{
public:

    UNITTEST( test_factory )
    {
        int a=2, b=1;

        std::string text;
        serl::save_archiver( new serl::text(text) )
        .serial("foo", a);

        serl::load_archiver( new serl::text(text) )
        .serial("foo", b);

        assertEqual(a, b);
    }


    UNITTEST( test_serial_primitives )
    {
        std::string storage;
        serl::text  nd("");

        serl::save_archiver arcv(new serl::text(storage));

        int ia=1, ib=0;
        arcv.serial("int", ia);

        std::string sa="test_string with \", =, \n and \\ chars", sb="none";

        char buf[] = " And even \0 (zero).";
        sa += std::string(buf, buf+sizeof(buf));

        arcv.serial("string", sa);

        double da=1.1, db=2.2;
        arcv.serial("doubles", da);

        arcv.close();

        serl::load_archiver ax(new serl::text(storage));

        ax.serial("int", ib);
        assertEqual(ia, ib);

        ax.serial("string", sb);
        assertEqual(sa, sb);

        ax.serial("doubles", db);
        assertEqual(da, db);
    }


    //////////////////////////////////////////////////////////////////////////

    struct test
    {
        int number;
        std::vector<int> vector;
        test(int x=0) : number(x), vector(x, 1) { }
        bool operator==(test other) const
        {
            return number==other.number && vector==other.vector;
        }

        void serialization(serl::archiver &ar)
        {
            ar.serial("int", number);
            ar.serial_container("vector", vector);
        }

        friend std::ostream &operator<<(std::ostream &stream, const test &self)
        {
            return stream << self.number;
        }
    };

    struct test_complex
    {
        test_complex(int i=0) : A(i) {}
        bool operator==(test_complex other) const
        {
            return A==other.A;
        }
        void serialization(serl::archiver &ar)
        {
            ar.serial("testA", A);
        }

        friend std::ostream &operator<<(std::ostream &stream, const test_complex &self)
        {
            return stream << self.A;
        }

        test  A;
    };

    UNITTEST( test_serial_classes )
    {
        std::string storage;

        test A(1), B(0);
        serl::save_archiver(new serl::text(storage)).serial("A", A);
        serl::load_archiver(new serl::text(storage)).serial("A", B);
        assertTrue(A==B);

        test_complex C(1), D(0);
        serl::save_archiver(new serl::text(storage)).serial("C", C);
        serl::load_archiver(new serl::text(storage)).serial("C", D);
        assertTrue(C==D);
    }

    //////////////////////////////////////////////////////////////////////////


    UNITTEST( test_default_values )
    {
        std::string source;

        int a=int(), b=1;
        serl::load_archiver(new serl::text(source)).serial("--", b);
        assertEqual(a, b);

        test_complex F, G(1);
        serl::load_archiver(new serl::text(source)).serial("--", G);
        assertTrue(F==G);
    }

    UNITTEST( test_serial_ptr )
    {
        std::string source;

        boost::shared_ptr<int> a(new int(1));
        boost::shared_ptr<int> b(new int(0));

        *b=0;
        serl::save_archiver(new serl::text(source)).serial_static_ptr("ptr2", a);
        serl::load_archiver(new serl::text(source)).serial_static_ptr("ptr2", b);
        assertEqual(*a, *b);

        std::vector<shared_ptr<int> > aa; aa.push_back(a);
        std::vector<shared_ptr<int> > bb;
        serl::save_archiver(new serl::text(source)).serial_static_ptr_container("vec", aa);
        serl::load_archiver(new serl::text(source)).serial_static_ptr_container("vec", bb);
        assertTrue(aa.size()==bb.size() && *aa[0]==*bb[0]);
    }

    //////////////////////////////////////////////////////////////////////////

    struct base_t
    {
        int number;
        base_t(int number) : number(number) {}
        virtual ~base_t() {}
        bool operator==(base_t other) const
        {
            return number==other.number;
        }

        void serialization(serl::archiver &ar)
        {
            ar.serial("number", number);
        }
    };

    struct derived_t : base_t
    {
        derived_t(int number=0) : base_t(number) {}
        bool operator==(derived_t other) const
        {
            return number==other.number;
        }

        void serialization(serl::archiver &ar)
        {
            base_t::serialization(ar);
        }
    };

    UNITTEST( test_virtual_ptr )
    {
        std::string source;

        serl::mark_as_rtti<derived_t> l;
        serl::mark_as_rtti_abstract<base_t> l1;

        base_t *a=(new derived_t(1));
        base_t *b=(new derived_t(0));

        serl::save_archiver(new serl::text(source)).serial_virtual_ptr("ptr", a);
        serl::load_archiver(new serl::text(source)).serial_virtual_ptr("ptr", b);
        assertTrue(*a==*b);

        shared_ptr<base_t> aa(new derived_t(1));
        shared_ptr<base_t> bb(new derived_t(0));
        serl::save_archiver(new serl::text(source)).serial_virtual_ptr("ptr2", aa);
        serl::load_archiver(new serl::text(source)).serial_virtual_ptr("ptr2", bb);
        assertTrue(*aa==*bb);
    }

    //////////////////////////////////////////////////////////////////////////

    UNITTEST( test_serial_container )
    {
        std::string source;

        std::vector<int> a(3, 1);
        std::vector<int> b(5, 0);
        serl::save_archiver(new serl::text(source)).serial_container("vector", a);
        serl::load_archiver(new serl::text(source)).serial_container("vector", b);

        assertTrue(a==b);
    }

    UNITTEST( test_serial_associate_container )
    {
        std::string source;

        std::map<int, double> a;
        a[1]=1.1; a[2]=1.2; a[3]=1.3;

        std::map<int, double> b;
        serl::save_archiver(new serl::text(source)).serial("vector", serl::make_container(a));
        serl::load_archiver(new serl::text(source)).serial("vector", serl::make_container(b));

        assertEqual(a.size(), b.size());
        assertEqual(a[1], b[1]);
        assertEqual(a[2], b[2]);
        assertEqual(a[3], b[3]);
    }

} INSTANCE;

//////////////////////////////////////////////////////////////

class UNITTEST_SUITE( serl_registry_test )
{

    void tearDown()
    {
        CRegKey tmp;
        tmp.Create(HKEY_CURRENT_USER, "Software\\AETestingTools");
        tmp.RecurseDeleteKey("Test");
    }

    UNITTEST( test_registry )
    {
        std::string a="test_string with \", =, " "\n and \\" " chars.", b="none";


        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial("name", a);

        serl::load_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial("name", b);

        assertEqual(a, b);
    }

    UNITTEST( test_seven )
    {
        int seven=7;
        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial("seven", seven);

        seven=1;
        serl::load_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial("seven", seven);

        assertEqual(7, seven);
    }
    UNITTEST( mru )
    {
        std::vector<std::string> sample;
        sample.push_back("D:\\work\\projects\\aera\\lib\\filter_sample.aera");
        sample.push_back("D:\\work\\projects\\aera\\lib\\q.aera");

        std::vector<std::string> vector;
        vector.push_back(sample[1]);
        vector.push_back(sample[0]);
        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial_container("MostRecentlyUsed", vector);

        // меняем местами
        std::swap(vector[0], vector[1]);
        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial_container("MostRecentlyUsed", vector);
        vector.clear();

        serl::load_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Test")).
        serial_container("MostRecentlyUsed", vector);

        assertEqual(sample.size(), vector.size());
        assertEqual(sample[0], vector[0]);
        assertEqual(sample[1], vector[1]);
    }
} INSTANCE;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( serl_data_node )
{
    UNITTEST( test_data_node_foo )
    {
        std::string storage;

        std::vector<std::string> v0;
        v0.push_back( "foo" );
        v0.push_back( "bar" );

        serl::save_archiver ax(new serl::data(storage));
        ax.serial_container("vector", v0);
        ax.close();

        std::string result;
        std::string sample;

        StringSource(storage, true,
                        new Gunzip(
                            new HexEncoder(
                                new StringSink(result),
                                true, 4
                            )));

        sample = "5200:0000:0000:0000:4A00:0000:0600:0000:7665:6374:"
                 "6F72:1200:0000:0500:0000:2373:697A:6505:0000:0001:"
                 "0000:0032:1100:0000:0200:0000:2330:0700:0000:0300:"
                 "0000:666F:6F11:0000:0002:0000:0023:3107:0000:0003:"
                 "0000:0062:6172";
        assertEqual(result, sample);

        result = "";
        StringSource(storage, true,
                        new Base64Encoder(
                             new StringSink(result)));
        sample = "H4sIAAAAAAAAAAtigAAvIGYD4rLU5JL8IiEgixWIlYszq1JBDEYgNhIEEkwgUQN2IMkMxGn5\n"
                 "+XBBQ5hgUmIRAE2r0nZWAAAA\n";
        assertEqual(result, sample);
    }

    UNITTEST( test_data_node1 )
    {
        std::string storage;

        std::string text="test", text2, text3;

        serl::save_archiver ax(new serl::data(storage));
        ax.serial("foo", text);
        ax.serial("bar", text);
        ax.close();

        serl::load_archiver bx(new serl::data(storage));
        bx.serial("foo", text2);
        bx.serial("bar", text3);

        assertEqual(text, text2);
        assertEqual(text, text3);
    }

    UNITTEST( test_data_node )
    {
        std::string storage;

        std::string text(500000, 'A'), text2;

        serl::save_archiver ax(new serl::data(storage));
        ax.serial("bar", text);
        ax.serial("foo", text);
        ax.close();

        serl::load_archiver bx(new serl::data(storage));
        bx.serial("bar", text2);
        bx.serial("foo", text2);

        assertTrue(text==text2);
    }
} INSTANCE;

