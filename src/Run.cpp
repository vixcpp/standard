/**
 *
 *  @file Run.cpp
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

#include <vix/Run.hpp>

/*
 * vix::run is intentionally implemented in Run.hpp.
 *
 * The operation is generic over the RunnableWith concept and preserves the
 * exact return type and value category of the underlying run() operation.
 * Its implementation must therefore remain available at the point of template
 * instantiation.
 *
 * This translation unit exists as the implementation boundary for the Run
 * component and may host non-template implementation details if the component
 * later requires them.
 */
