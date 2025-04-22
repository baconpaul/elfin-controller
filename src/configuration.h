//
// Created by Paul Walker on 4/22/25.
//

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

namespace baconpaul::elfin
{
static constexpr int numElfinParams{20};

#define ELFLOG(...) std::cout << __FILE__ << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl;
};
#endif //CONFIGURATION_H
