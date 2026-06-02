// Java + libGDX: configure a FitViewport to preserve aspect ratio automatically.
import com.badlogic.gdx.utils.viewport.FitViewport;
import com.badlogic.gdx.graphics.OrthographicCamera;

public final class AspectFit {
    public static final float LOGICAL_WIDTH = 960f;
    public static final float LOGICAL_HEIGHT = 720f;

    public static FitViewport createViewport() {
        return new FitViewport(LOGICAL_WIDTH, LOGICAL_HEIGHT, new OrthographicCamera());
    }

    public static void resize(FitViewport viewport, int width, int height) {
        viewport.update(width, height, true);
    }
}
