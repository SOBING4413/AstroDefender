// AstroDefender polyglot module: Java asset_pipeline
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Validates asset metadata before the C runtime and other language modules consume it.
import com.badlogic.gdx.utils.viewport.FitViewport;
import com.badlogic.gdx.graphics.OrthographicCamera;
import java.util.Map;

public final class AstroAssetPipeline {
    public static final String CONTRACT_VERSION = "1.0.0";
    public static final float LOGICAL_WIDTH = 960f;
    public static final float LOGICAL_HEIGHT = 720f;

    private AstroAssetPipeline() {}

    public static FitViewport createSharedViewport() {
        return new FitViewport(LOGICAL_WIDTH, LOGICAL_HEIGHT, new OrthographicCamera());
    }

    public static boolean assetManifestMatchesContract(Map<String, String> manifest) {
        return CONTRACT_VERSION.equals(manifest.get("contract_version"))
            && "960".equals(manifest.get("logical_width"))
            && "720".equals(manifest.get("logical_height"));
    }
}
