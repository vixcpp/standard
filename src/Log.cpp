/**
 *
 *  @file Log.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/vixcpp/standard
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Vix.cpp Standard
 */

#include <vix/Log.hpp>

/*
 * vix::log is intentionally implemented in Log.hpp.
 *
 * The operation is generic over the LogTarget concept and preserves the exact
 * return type of the underlying log() operation. Its implementation therefore
 * remains available at the point of template instantiation.
 *
 * This translation unit exists as the implementation boundary for the Log
 * component and may host non-template implementation details if the component
 * later requires them.
 */
