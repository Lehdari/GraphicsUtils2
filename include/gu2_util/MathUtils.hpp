//
// Project: GraphicsUtils2
// File: MathUtils.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <nlohmann/json.hpp>


// Json serializer specializations for Eigen types
NLOHMANN_JSON_NAMESPACE_BEGIN
template <typename T_Scalar, int T_Rows, int T_Cols>
struct adl_serializer<Eigen::Matrix<T_Scalar, T_Rows, T_Cols>> {
    static void to_json(json& json, const Eigen::Matrix<T_Scalar, T_Rows, T_Cols>& m) {
        std::array<T_Scalar, T_Rows*T_Cols> arr;
        for (int i=0; i<T_Cols; ++i) {
            for (int j=0; j<T_Rows; ++j) {
                arr[4*i + j] = m(j,i);
            }
        }
        json = std::move(arr);
    }

    static void from_json(const json& json, Eigen::Matrix<T_Scalar, T_Rows, T_Cols>& m) {
        for (int i=0; i<T_Cols; ++i) {
            for (int j=0; j<T_Rows; ++j) {
                m(j,i) = json[4*i + j];
            }
        }
    }
};

template <typename T_Scalar>
struct adl_serializer<Eigen::Quaternion<T_Scalar>> {
    static void to_json(json& json, const Eigen::Quaternion<T_Scalar>& q) {
        std::array<T_Scalar, 4> arr;
        arr[0] = q.x();
        arr[1] = q.y();
        arr[2] = q.z();
        arr[3] = q.w();
        json = std::move(arr);
    }

    static void from_json(const json& json, Eigen::Quaternion<T_Scalar>& q) {
        q.x() = json[0];
        q.y() = json[1];
        q.z() = json[2];
        q.w() = json[3];
    }
};
NLOHMANN_JSON_NAMESPACE_END
