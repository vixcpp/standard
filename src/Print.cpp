/**
 *
 *  @file Print.cpp
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

#include <vix/Print.hpp>

/*
 * vix::print is intentionally implemented in Print.hpp.
 *
 * The operation is generic over the Output concept and preserves the
 * destination-specific error type. Its implementation therefore remains
 * available at the point of template instantiation.
 *
 * This translation unit exists as the implementation boundary for the Print
 * component and may host non-template implementation details if the component
 * later requires them.
 */
