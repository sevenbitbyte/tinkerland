#ifndef PERSON_HXX
#define PERSON_HXX

// person.hxx
#include <string>

#include <odb/core.hxx>



#pragma db object
class person
{
public:
  person (const std::string& first,
          const std::string& last,
          unsigned short age);

  const std::string& getFirst () const;
  const std::string& getLast () const;
  unsigned short getAge () const;
  void setAge(unsigned short value);

private:
  person () {}

  friend class odb::access;

  #pragma db id auto
  unsigned long id;

  std::string first;
  std::string last;
  unsigned short age;
};


#endif	//PERSON_HXX

