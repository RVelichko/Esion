#pragma once

#include "NonCopyable.hpp"

namespace Common
{
  
  class BoolFlagInvertor final : private utils::NonCopyable<BoolFlagInvertor> {
  public:
    BoolFlagInvertor(bool volatile *flag)
      : Flag(flag)
    {
    }
    ~BoolFlagInvertor()
    {
      *Flag = !*Flag;
    }
    
  private:
    bool volatile *Flag;
  };
  
}
