#include "parallelismconstraint.h"
#include "../edge.h"
#include "../vertex.h"

ParallelismConstraint::ParallelismConstraint(Edge* e1, Edge* e2) {
    parallel_edges[0] = e1;
    parallel_edges[1] = e2;
}

const Edge* ParallelismConstraint::GetEdge1() const {
    return parallel_edges[0];
}

const Edge* ParallelismConstraint::GetEdge2() const {
    return parallel_edges[1];
}

std::optional<std::unique_ptr<ParallelismConstraint>> ParallelismConstraint::Create(Edge* e1, Edge* e2) {
    //correct validation
    if(e1->vertices.first == e2->vertices.first || e1->vertices.first == e2->vertices.second || e1->vertices.second == e2->vertices.first || e1->vertices.second == e2->vertices.second)
        return std::nullopt;
    else return std::unique_ptr<ParallelismConstraint>(new ParallelismConstraint(e1, e2));
}

void ParallelismConstraint::Fix(const Vertex* theImmovableVertex) const {
    int immovable_edge_index = -1;
    if(parallel_edges[0]->vertices.first.get() == theImmovableVertex || parallel_edges[0]->vertices.second.get() == theImmovableVertex)
    {
        immovable_edge_index = 0;
    }
    else if(parallel_edges[1]->vertices.first.get() == theImmovableVertex || parallel_edges[1]->vertices.second.get() == theImmovableVertex) {
        immovable_edge_index = 1;
    }

    if(immovable_edge_index != -1) {
        int movable_edge_index = 1 - immovable_edge_index;
        auto immovableEdge = parallel_edges[immovable_edge_index];
        auto movableEdge = parallel_edges[movable_edge_index];

        QPointF p1_imm = immovableEdge->vertices.first->point;
        QPointF p2_imm = immovableEdge->vertices.second->point;

        QPointF p1_mov = movableEdge->vertices.first->point;
        QPointF p2_mov = movableEdge->vertices.second->point;

        //vectors
        QPointF v1 = p2_imm - p1_imm;
        QPointF v2 = p2_mov - p1_mov;

        double dotProduct = v1.x() * v2.x() + v1.y() * v2.y();

        // Target angle dependent on the orientation of the vectors
        double angle1 = std::atan2(v1.y(), v1.x());
        double targetAngle = (dotProduct >= 0.0) ? angle1 : angle1 + M_PI;

        // The middle of the movable edge
        QPointF midpoint = (p1_mov + p2_mov) / 2.0;
        double halfLength = std::hypot(v2.x(), v2.y()) / 2.0;

        // Displacement vector from the middle of the vector
        QPointF offset(halfLength * std::cos(targetAngle), halfLength * std::sin(targetAngle));
        movableEdge->vertices.second->point = midpoint + offset;
        movableEdge->vertices.first->point = midpoint - offset;
    }
    else {
        auto edge0 = parallel_edges[0];
        auto edge1 = parallel_edges[1];

        Vertex* v0_start = edge0->vertices.first.get();
        Vertex* v0_end = edge0->vertices.second.get();

        Vertex* v1_start = edge1->vertices.first.get();
        Vertex* v1_end = edge1->vertices.second.get();

        QPointF dir0 = v0_end->point - v0_start->point;
        QPointF dir1 = v1_end->point - v1_start->point;

        double dotProduct = dir0.x() * dir1.x() + dir0.y() * dir1.y();

        // Align edge1 vertices if vectors point in opposite directions
        if (dotProduct < 0.0) {
            std::swap(v1_start, v1_end);
        }

        QPointF dir1_aligned = v1_end->point - v1_start->point;

        double angle0 = std::atan2(dir0.y(), dir0.x());
        double angle1 = std::atan2(dir1_aligned.y(), dir1_aligned.x());

        // Average angle using sine/cosine mean to avoid wrap-around issues
        double avgCos = (std::cos(angle0) + std::cos(angle1)) / 2.0;
        double avgSin = (std::sin(angle0) + std::sin(angle1)) / 2.0;
        double avgAngle = std::atan2(avgSin, avgCos);

        // Midpoints and half lengths
        QPointF mid0 = (v0_start->point + v0_end->point) / 2.0;
        double halfLen0 = std::hypot(dir0.x(), dir0.y()) / 2.0;

        QPointF mid1 = (v1_start->point + v1_end->point) / 2.0;
        double halfLen1 = std::hypot(dir1_aligned.x(), dir1_aligned.y()) / 2.0;

        QPointF offset0(halfLen0 * std::cos(avgAngle), halfLen0 * std::sin(avgAngle));
        QPointF offset1(halfLen1 * std::cos(avgAngle), halfLen1 * std::sin(avgAngle));

        // Rotate both edges around their midpoints using aligned direction
        v0_start->point = mid0 - offset0;
        v0_end->point = mid0 + offset0;

        v1_start->point = mid1 - offset1;
        v1_end->point = mid1 + offset1;
    }
}

float ParallelismConstraint::getError() {
    QPointF v1 = parallel_edges[0]->vertices.second->point - parallel_edges[0]->vertices.first->point;
    QPointF v2 = parallel_edges[1]->vertices.second->point - parallel_edges[1]->vertices.first->point;

    // Determine mutual orientation
    double dotProduct = v1.x() * v2.x() + v1.y() * v2.y();

    double angle1 = std::atan2(v1.y(), v1.x());
    double angle2 = std::atan2(v2.y(), v2.x());

    if (dotProduct < 0.0) {
        angle2 += M_PI; // Align direction if edges run oppositely
    }

    double diff = angle1 - angle2;

    // Normalize diff to range (-PI, PI]
    while (diff > M_PI)  diff -= 2.0 * M_PI;
    while (diff <= -M_PI) diff += 2.0 * M_PI;

    return std::abs(diff); // Error in radians (0.0 means perfectly parallel)
}

std::vector<const Vertex *> ParallelismConstraint::affectedVertices() const {
    std::vector<const Vertex *> result;
    result.push_back(parallel_edges[0]->vertices.first.get());
    result.push_back(parallel_edges[0]->vertices.second.get());
    auto temp_v = parallel_edges[1]->vertices.first.get();
    if(temp_v != result[0] && temp_v != result[1]) {
        result.push_back(temp_v);
    }
    temp_v = parallel_edges[1]->vertices.second.get();
    if(temp_v != result[0] && temp_v != result[1]) {
        result.push_back(temp_v);
    }
    return result;
}