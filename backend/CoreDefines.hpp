#pragma once
#ifndef BACKEND_COREDEFINES_HPP_
#define BACKEND_COREDEFINES_HPP_

#define BACKEND_NAMESPACE backend
#define HOMESITE_NAMESPACE home
#define NOISEGENERATOR_NAMSEPACE noise_gen

#ifdef LUNUX

#define TEMP_FOLDER "\\tmp\\"

#elif defined(_WIN32) || defined(WIN32)

#define TEMP_FOLDER ""

#endif // LUNUX


#endif // !BACKEND_COREDEFINES_HPP_