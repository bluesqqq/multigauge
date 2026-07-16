import { createFieldShell } from "./shared.js";

const IMAGE_FILE_ACCEPT = "image/png,image/jpeg,image/bmp,.png,.jpg,.jpeg,.bmp";

function looksLikeImagePathMeta(meta) {
	if (!meta || typeof meta !== "object") return false;
	if (meta.key !== "path") return false;
	const labelText = `${meta.name || ""} ${meta.description || ""}`.toLowerCase();
	return labelText.includes("image");
}

function sanitizeAssetFilename(filename, fallback = "image.png") {
	const raw = String(filename || fallback).trim() || fallback;
	const safe = raw.replace(/[^a-z0-9._-]+/gi, "_").replace(/^_+|_+$/g, "");
	return safe || fallback;
}

function ensureUniqueAssetName(filename, assets) {
	const safeName = sanitizeAssetFilename(filename);
	const lastDot = safeName.lastIndexOf(".");
	const base = lastDot > 0 ? safeName.slice(0, lastDot) : safeName;
	const ext = lastDot > 0 ? safeName.slice(lastDot) : "";
	const existing = new Set((Array.isArray(assets) ? assets : []).map((asset) => String(asset?.name || "")));
	if (!existing.has(safeName)) return safeName;

	let counter = 2;
	while (true) {
		const candidate = `${base}_${counter}${ext}`;
		if (!existing.has(candidate)) return candidate;
		counter += 1;
	}
}

function readFileAsDataUrl(file) {
	return new Promise((resolve, reject) => {
		const reader = new FileReader();
		reader.onload = () => resolve(typeof reader.result === "string" ? reader.result : "");
		reader.onerror = () => reject(reader.error || new Error("Failed to read file"));
		reader.readAsDataURL(file);
	});
}

function parseDataUrl(dataUrl) {
	const match = /^data:([^;,]+);base64,(.+)$/i.exec(String(dataUrl || ""));
	if (!match) return null;
	return {
		mediaType: match[1],
		data: match[2],
	};
}

function isSupportedMediaType(mediaType) {
	return ["image/png", "image/jpeg", "image/bmp"].includes(String(mediaType || "").toLowerCase());
}

function findAssetByName(assets, name) {
	return (Array.isArray(assets) ? assets : []).find((asset) => String(asset?.name || "") === String(name || "")) || null;
}

function createPreviewDataUrl(asset) {
	if (!asset?.mediaType || !asset?.data) return "";
	return `data:${asset.mediaType};base64,${asset.data}`;
}

export function renderImageAssetWidget({
	container,
	meta,
	path,
	targetId,
	setPropertyValue,
	getDocumentAssets,
	setDocumentAssets,
}) {
	if (!looksLikeImagePathMeta(meta)) {
		return false;
	}

	const assetsManagerAvailable = typeof getDocumentAssets === "function" && typeof setDocumentAssets === "function";
	const assets = assetsManagerAvailable && Array.isArray(getDocumentAssets?.()) ? getDocumentAssets() : [];
	const currentValue = meta.value == null ? "" : String(meta.value);
	const currentAsset = findAssetByName(assets, currentValue);

	const field = createFieldShell(meta, true);
	const widget = document.createElement("div");
	widget.className = "imageAssetWidget";

	const inputRow = document.createElement("div");
	inputRow.className = "imageAssetWidgetInputRow";

	const input = document.createElement("input");
	input.type = "text";
	input.value = currentValue;
	input.placeholder = "Embedded asset name or image path";

	const applyButton = document.createElement("button");
	applyButton.type = "button";
	applyButton.className = "workspaceToolButton imageAssetWidgetButton";
	applyButton.textContent = "Apply";

	inputRow.appendChild(input);
	inputRow.appendChild(applyButton);
	widget.appendChild(inputRow);

	const actionsRow = document.createElement("div");
	actionsRow.className = "imageAssetWidgetActions";

	let uploadButton = null;
	let fileInput = null;
	if (assetsManagerAvailable) {
		uploadButton = document.createElement("button");
		uploadButton.type = "button";
		uploadButton.className = "workspaceToolButton imageAssetWidgetButton";
		uploadButton.textContent = "Upload";
		actionsRow.appendChild(uploadButton);
	}

	if (assetsManagerAvailable && assets.length) {
		const select = document.createElement("select");
		select.className = "imageAssetWidgetSelect";

		const placeholder = document.createElement("option");
		placeholder.value = "";
		placeholder.textContent = "Choose embedded image";
		select.appendChild(placeholder);

		for (const asset of assets) {
			if (!asset?.name) continue;
			const option = document.createElement("option");
			option.value = asset.name;
			option.textContent = asset.name;
			if (asset.name === currentValue) option.selected = true;
			select.appendChild(option);
		}

		select.addEventListener("change", () => {
			if (!select.value) return;
			input.value = select.value;
			setPropertyValue(targetId, path, select.value, { reloadDocument: true });
		});

		actionsRow.appendChild(select);
	}

	widget.appendChild(actionsRow);

	const hint = document.createElement("div");
	hint.className = "imageAssetWidgetHint";
	hint.textContent = assetsManagerAvailable
		? "Supported uploads: PNG, JPEG, BMP."
		: "Embedded asset management is unavailable in this build.";
	widget.appendChild(hint);

	if (currentAsset) {
		const preview = document.createElement("img");
		preview.className = "imageAssetWidgetPreview";
		preview.src = createPreviewDataUrl(currentAsset);
		preview.alt = currentAsset.name || "Embedded image preview";
		widget.appendChild(preview);
	}

	applyButton.addEventListener("click", () => {
		setPropertyValue(targetId, path, input.value, { reloadDocument: true });
	});

	input.addEventListener("keydown", (event) => {
		if (event.key === "Enter") {
			event.preventDefault();
			setPropertyValue(targetId, path, input.value, { reloadDocument: true });
		}
	});

	if (uploadButton) {
		fileInput = document.createElement("input");
		fileInput.type = "file";
		fileInput.accept = IMAGE_FILE_ACCEPT;
		fileInput.className = "hidden";

		uploadButton.addEventListener("click", () => {
			fileInput.click();
		});

		fileInput.addEventListener("change", async () => {
			const file = fileInput.files?.[0];
			fileInput.value = "";
			if (!file) return;

			try {
				const dataUrl = await readFileAsDataUrl(file);
				const parsed = parseDataUrl(dataUrl);
				if (!parsed || !isSupportedMediaType(parsed.mediaType)) {
					throw new Error("Unsupported image type");
				}

				const nextAssetName = ensureUniqueAssetName(file.name, assets);
				const nextAssets = assets.filter((asset) => String(asset?.name || "") !== nextAssetName);
				nextAssets.push({
					name: nextAssetName,
					mediaType: parsed.mediaType,
					data: parsed.data,
				});

				await setDocumentAssets(nextAssets);
				input.value = nextAssetName;
				await setPropertyValue(targetId, path, nextAssetName, { reloadDocument: true });
			} catch (error) {
				alert(`Failed to upload image: ${error.message || error}`);
			}
		});
	}

	field.appendChild(widget);
	if (fileInput) field.appendChild(fileInput);
	container.appendChild(field);
	return true;
}
