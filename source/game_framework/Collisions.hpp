#ifndef COLLISSIONS_HPP
#define COLLISSIONS_HPP

#include "Rect.hpp"
#include "Vector.hpp"

#define BIT(x) (1 << (x))

enum class ECollisionTag : int
{
    NONE   = 0,                 //! < No collision detected
    CELL   = BIT(0),            //! < Collision with top side detected
    FLOOR  = BIT(1),            //! < Collision with bottom side detected
    LEFT   = BIT(2),            //! < Collision with left side detected
    RIGHT  = BIT(3),            //! < Collision with right side detected
    UP     = CELL,
    DOWN   = FLOOR,
    X_AXIS = (LEFT | RIGHT),    //! < Collision on X axis detected (left or right)
    Y_AXIS = (UP | DOWN),       //! < Collision on Y axis detected (top or bottom)
};

//---------------------------------------------------------------------------
//!
//! \brief Bitwise OR operator for collision tags.
//!
//! \param one [in] - first collision tag
//! \param two [in] - second collision tag
//!
//! \return Result of bitwise OR operation
//!
ECollisionTag operator |= (ECollisionTag& one, const ECollisionTag& two);

//---------------------------------------------------------------------------
//!
//! \brief Bitwise AND operator for collision tags.
//!
//! \param one [in] - first collision tag
//! \param two [in] - second collision tag
//!
//! \returns True if both tags are set
//!
bool operator & (ECollisionTag one, ECollisionTag two);

//---------------------------------------------------------------------------
//!
//! \brief Calculates collision response.
//!
//! \param own_rect      [in]  - own bounding rectangle
//! \param own_speed     [in]  - own speed vector
//! \param other_rect    [in]  - other bounding rectangle
//! \param other_speed   [in]  - other speed vector
//! \param delta_time    [in]  - time delta since last frame
//! \param collision_tag [out] - collision tags result
//!
//! \return New position for the object
//!
Vector collisionResponse(const Rect& own_rect, const Vector& own_speed,
                         const Rect& other_rect, const Vector& other_speed,
                         const float delta_time, ECollisionTag& collision_tag);

#endif // !COLLISSIONS_HPP
