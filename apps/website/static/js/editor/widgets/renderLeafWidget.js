import { renderBoolWidget } from "./boolWidget.js";
import { renderColorSelectorWidget } from "./colorSelectorWidget.js";
import { renderColorWidget } from "./colorWidget.js";
import { renderEnumWidget } from "./enumWidget.js";
import { renderGradientWidget } from "./gradientWidget.js";
import { renderImageAssetWidget } from "./imageAssetWidget.js";
import { inferLeafKind } from "./shared.js";
import { renderJsonWidget } from "./jsonWidget.js";
import { renderJustifyWidget } from "./justifyWidget.js";
import { createLayoutSizeEditor } from "./layoutSizeWidget.js";
import { renderNumberWidget } from "./numberWidget.js";
import { createFieldShell } from "./shared.js";
import { renderTextWidget } from "./textWidget.js";
import { renderValueWidget } from "./valueWidget.js";

export function renderLeafWidget({
	container,
	meta,
	path,
	targetId,
	setPropertyValue,
	openColorPanel,
	state,
	getDocumentAssets,
	setDocumentAssets,
}) {
	const kind = inferLeafKind(meta);

	if ((meta.widget || "") === "layout-size") {
		const field = createFieldShell(meta, false);
		const row = createLayoutSizeEditor({
			value: meta.value,
			onCommit: (nextValue) => setPropertyValue(targetId, path, nextValue),
		});
		field.appendChild(row);
		container.appendChild(field);
		return;
	}

	if ((meta.widget || "") === "justify") {
		if (renderJustifyWidget({ container, meta, path, targetId, setPropertyValue })) return;
	}

	if ((meta.widget || "") === "enum" || (meta.widget || "") === "select" || (meta.widget || "") === "segmented-select") {
		if (renderEnumWidget({ container, meta, path, targetId, setPropertyValue })) return;
	}

	if ((meta.widget || "") === "value") {
		if (renderValueWidget({ container, meta, path, targetId, setPropertyValue, state })) return;
	}

	if (kind === "bool") {
		renderBoolWidget({ container, meta, path, targetId, setPropertyValue });
		return;
	}

	if (kind === "number") {
		renderNumberWidget({ container, meta, path, targetId, setPropertyValue });
		return;
	}

	if (kind === "json") {
		renderJsonWidget({ container, meta, path, targetId, setPropertyValue });
		return;
	}

	if (kind === "color") {
		renderColorWidget({ container, meta, path, targetId, openColorPanel });
		return;
	}

	if (kind === "color-selector") {
		renderColorSelectorWidget({ container, meta, path, targetId, setPropertyValue });
		return;
	}

	if (kind === "gradient") {
		renderGradientWidget({ container, meta, path, targetId, setPropertyValue });
		return;
	}

	if (
		renderImageAssetWidget({
			container,
			meta,
			path,
			targetId,
			setPropertyValue,
			getDocumentAssets,
			setDocumentAssets,
		})
	) {
		return;
	}

	renderTextWidget({ container, meta, path, targetId, setPropertyValue });
}
