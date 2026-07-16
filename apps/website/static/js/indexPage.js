function applyHeroModelMaterials(heroModel) {
    const materials = heroModel.model?.materials ?? [];

    for (const material of materials) {
        const pbr = material.pbrMetallicRoughness;
        pbr.setBaseColorFactor([0, 0, 0, 1]);
        pbr.setMetallicFactor(0);
        pbr.setRoughnessFactor(1);

        if (material.setEmissiveFactor) {
            material.setEmissiveFactor([0, 0, 0]);
        }
    }
}

document.addEventListener("DOMContentLoaded", () => {
    const heroModel = document.getElementById("heroModel");
    if (!heroModel) return;

    const updateMaterials = () => applyHeroModelMaterials(heroModel);

    if (heroModel.model) {
        updateMaterials();
        return;
    }

    heroModel.addEventListener("load", updateMaterials, { once: true });
});
