import React, { useEffect, useState } from 'react';
import { DocArticle } from '../data/docsContent';
import { NAV_STRUCTURE } from '../data/docsNav';
import { Copy, Check, ArrowLeft, ArrowRight } from 'lucide-react';
import { SupportedLanguage, TranslationStrings } from '../i18n/translations';
import { getLocalizedArticleTitle } from '../i18n/articleTranslations';

interface DocContentProps {
  article: DocArticle;
  onNavigate: (id: string) => void;
  language: SupportedLanguage;
  t: TranslationStrings;
}

export const DocContent: React.FC<DocContentProps> = ({
  article,
  onNavigate,
  language,
  t,
}) => {
  const [copiedPage, setCopiedPage] = useState(false);

  // Flatten navigation items to calculate previous & next article
  const allNavItems: { id: string; title: string }[] = [];
  NAV_STRUCTURE.forEach((cat) => {
    cat.items.forEach((item) => {
      allNavItems.push(item);
    });
  });

  const currentIndex = allNavItems.findIndex((item) => item.id === article.id);
  const prevItem = currentIndex > 0 ? allNavItems[currentIndex - 1] : null;
  const nextItem = currentIndex < allNavItems.length - 1 ? allNavItems[currentIndex + 1] : null;

  // Copy entire page content to clipboard
  const handleCopyPage = () => {
    const rawContent = `# ${localizedTitle}\nCategory: ${article.category}\nLast Updated: ${article.lastUpdated}\n\n${article.summary}\n\n` +
      article.body.replace(/<[^>]*>?/gm, '').replace(/\n{3,}/g, '\n\n');
    navigator.clipboard.writeText(rawContent.trim());
    setCopiedPage(true);
    setTimeout(() => setCopiedPage(false), 2000);
  };

  useEffect(() => {
    // Enhance code blocks with copy button
    const preBlocks = document.querySelectorAll('.doc-body-content pre');
    preBlocks.forEach((pre) => {
      if (pre.querySelector('.code-header-bar')) return;

      const codeEl = pre.querySelector('code');
      let lang = 'bash';
      if (codeEl) {
        const className = codeEl.className || '';
        const match = className.match(/language-(\w+)/);
        if (match) lang = match[1];
      }

      const headerBar = document.createElement('div');
      headerBar.className = 'code-header-bar';
      headerBar.innerHTML = `
        <span class="code-lang-label">${lang}</span>
        <button class="code-copy-btn" aria-label="Copy code">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
          <span>${t.copy}</span>
        </button>
      `;

      const copyBtn = headerBar.querySelector('.code-copy-btn');
      if (copyBtn) {
        copyBtn.addEventListener('click', () => {
          const text = codeEl?.innerText || pre.textContent || '';
          navigator.clipboard.writeText(text.trim());
          copyBtn.innerHTML = `
            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="#00A8B5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg>
            <span style="color: #00A8B5; font-weight: 500;">${t.copied}</span>
          `;
          setTimeout(() => {
            copyBtn.innerHTML = `
              <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/></svg>
              <span>${t.copy}</span>
            `;
          }, 2000);
        });
      }

      pre.insertBefore(headerBar, pre.firstChild);
    });
  }, [article, t]);

  const localizedTitle = getLocalizedArticleTitle(article.id, article.title, language);

  return (
    <article className="claude-article">
      <header className="article-title-header">
        <div className="title-row">
          <h1 className="article-serif-title">{localizedTitle}</h1>
          <button onClick={handleCopyPage} className="copy-llm-btn" aria-label="Copy page content">
            {copiedPage ? <Check size={14} className="copied-icon" /> : <Copy size={14} />}
            <span>{copiedPage ? t.copiedPage : t.copyPage}</span>
          </button>
        </div>
        <p className="article-publish-date">{article.lastUpdated}</p>
      </header>

      {article.summary && (
        <p className="article-lead-intro">{article.summary}</p>
      )}

      <div
        className="doc-body-content"
        dangerouslySetInnerHTML={{ __html: article.body }}
      />

      <footer className="article-bottom-pager">
        <div className="pager-container">
          {prevItem ? (
            <button
              onClick={() => onNavigate(prevItem.id)}
              className="claude-pager-card prev-card"
            >
              <ArrowLeft size={16} className="pager-arrow" />
              <div className="pager-meta">
                <span className="pager-label">{t.previous}</span>
                <span className="pager-title">{getLocalizedArticleTitle(prevItem.id, prevItem.title, language)}</span>
              </div>
            </button>
          ) : <div className="pager-spacer" />}

          {nextItem && (
            <button
              onClick={() => onNavigate(nextItem.id)}
              className="claude-pager-card next-card"
            >
              <div className="pager-meta text-right">
                <span className="pager-label">{t.next}</span>
                <span className="pager-title">{getLocalizedArticleTitle(nextItem.id, nextItem.title, language)}</span>
              </div>
              <ArrowRight size={16} className="pager-arrow" />
            </button>
          )}
        </div>
      </footer>
    </article>
  );
};
