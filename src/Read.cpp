/**
 *
 *  @file Read.cpp
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

#include <vix/Read.hpp>

/*
 * vix::read is intentionally implemented in Read.hpp.
 *
 * The operation is generic over the Input concept and must remain available
 * at the point of template instantiation. This translation unit exists as the
 * implementation boundary for the Read component and may host non-template
 * implementation details if the public operation later requires them.
 */
