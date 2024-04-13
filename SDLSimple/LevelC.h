#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 4;

    ~LevelC();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
