// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"
#include <vector>

namespace ECS {
    template <typename Element, typename Vector = std::vector<Element>>
    flecs::opaque<Vector, Element> VectorReflection(flecs::world& world) {
        std::stringstream ss;
        ss << "vector_";
        ss << world.component<Element>().name();
        flecs::entity v = world.vector<Element>();
        v.set_name(ss.str().c_str());

        return flecs::opaque<Vector, Element>()
            .as_type(v)

            // Forward elements of std::vector value to serializer
            .serialize([](const flecs::serializer* s, const Vector* data) {
            for (const auto& el : *data) {
                s->value(el);
            }
            return 0;
                })

            // Return vector count
            .count([](const Vector* data) {
            return data->size();
                })

            // Resize contents of vector
            .resize([](Vector* data, size_t size) {
            data->resize(size);
                })

            // Ensure element exists, return pointer
            .ensure_element([](Vector* data, size_t elem) {
            if (data->size() <= elem) {
                data->resize(elem + 1);
            }

            return &data->data()[elem];
                });
    }
}