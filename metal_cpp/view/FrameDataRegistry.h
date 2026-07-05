//
//  FrameDataRegistry.h
//  metal_cpp
//
//  Created by Neel on 04/07/26.
//

#pragma once

#include <typeindex>
#include <any>
#include <map>

class FrameDataRegistry{
public:
    template<typename T>
    void publish(T&& value){
        using CleanType = std::decay_t<T>;
        data[typeid(CleanType)] = std::forward<T>(value);
    }

    template<typename T>
    const T& get() const{
        return std::any_cast<const T&>(data.at(typeid(T)));
    }
    
private:
    std::unordered_map<std::type_index, std::any> data;
};
