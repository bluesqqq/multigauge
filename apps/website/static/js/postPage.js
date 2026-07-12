function bindJsonForm(formId, onSuccess, errorLabel) {
    const form = document.getElementById(formId);
    if (!form) return;

    form.addEventListener("submit", async (event) => {
        event.preventDefault();

        try {
            const response = await fetch(form.action, {
                method: "POST",
                body: new FormData(form),
                headers: {
                    Accept: "application/json",
                },
            });

            onSuccess(await response.json());
        } catch (error) {
            console.error(errorLabel, error);
        }
    });
}

function updateToggleImage(imageId, active) {
    const image = document.getElementById(imageId);
    if (!image) return;

    const nextSrc = active ? image.dataset.activeSrc : image.dataset.inactiveSrc;
    if (nextSrc) {
        image.src = nextSrc;
    }
}

document.addEventListener("DOMContentLoaded", () => {
    bindJsonForm("like-form", (data) => {
        updateToggleImage("like-image", !!data.liked);

        const likeCount = document.getElementById("like-count");
        if (likeCount) {
            likeCount.textContent = `${data.total_likes} likes`;
        }
    }, "[post] Like update failed");

    bindJsonForm("favorite-form", (data) => {
        updateToggleImage("favorite-image", !!data.favorited);

        const favoriteCount = document.getElementById("favorite-count");
        if (favoriteCount) {
            favoriteCount.textContent = `${data.total_favorites} favorites`;
        }
    }, "[post] Favorite update failed");

    bindJsonForm("feature-form", (data) => {
        updateToggleImage("feature-image", !!data.featured);
    }, "[post] Feature update failed");
});
